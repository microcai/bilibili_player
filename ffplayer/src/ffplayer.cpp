
#include <QtCore>
#include <QMediaService>

#include "ffmpeg.hpp"
#include "ffplayer.hpp"
#include "ffplayer_p.hpp"
#include "qio2avio.hpp"

FFPlayerPrivate::FFPlayerPrivate(FFPlayer* q)
	: q_ptr(q)
	, vdecoder(q)
	, avsync(q)
{
	avcodec_register_all();
	av_register_all();

	avformat_network_init();
}

FFPlayerPrivate::~FFPlayerPrivate()
{
	delete demuxer;
	avformat_network_deinit();
}

FFPlayer::FFPlayer()
 : d_ptr(new FFPlayerPrivate(this))
{
	demux_thread.start();

	d_func()->vdecoder.moveToThread(&demux_thread);
	d_func()->adecoder.moveToThread(&demux_thread);
	d_func()->avsync.moveToThread(&demux_thread);

	connect(&d_ptr->avsync, SIGNAL(render_frame(const QVideoFrame&)), this, SLOT(render_frame(const QVideoFrame&)), Qt::DirectConnection);

	connect(&d_ptr->avsync, &QAudioVideoSync::render_frame, this, [this](const QVideoFrame& f)
	{
		if (f.startTime() /1000.0  >= ((d_ptr->avformat_ctx->duration / (double)AV_TIME_BASE)) - 5 )
		{
			std::thread(&FFPlayer::preload_next_uri, this).detach();
		}

	}, Qt::DirectConnection);



	connect(&d_ptr->vdecoder, SIGNAL(videoframe_decoded(const QVideoFrame&)), &d_ptr->avsync, SLOT(sync_frame(const QVideoFrame&)), Qt::DirectConnection);
	connect(&d_ptr->adecoder, SIGNAL(audioframe_decoded(const QAudioBuffer&)), &d_ptr->avsync, SLOT(sync_audio(const QAudioBuffer&)), Qt::DirectConnection);

	connect(&d_ptr->avsync, &QAudioVideoSync::nomore_frames, this, [this]()
	{
		m_MediaStatus = QMediaPlayer::MediaStatus::BufferingMedia;
		mediaStatusChanged(m_MediaStatus);
	});

	connect(&d_ptr->avsync, &QAudioVideoSync::frames_ready, this, [this]()
	{
		if ( MediaStatus() != QMediaPlayer::MediaStatus::BufferedMedia)
		{
			m_MediaStatus = QMediaPlayer::MediaStatus::BufferedMedia;
			mediaStatusChanged(m_MediaStatus);
		}
	});

	connect(&d_ptr->avsync, &QAudioVideoSync::suspended, this, [this](){
		m_state = QMediaPlayer::PausedState;
		stateChanged(m_state);
	});

	connect(&d_ptr->avsync, &QAudioVideoSync::running, this, [this](){
		m_state = QMediaPlayer::PlayingState;
		stateChanged(m_state);
	});

	auto finish_connect = connect(&d_ptr->avsync, &QAudioVideoSync::play_finished, this, [this]()
	{
		if ( m_state != QMediaPlayer::PlayingState)
		{
			setProperty("MediaStatus", QMediaPlayer::MediaStatus::EndOfMedia);
			mediaStatusChanged(QMediaPlayer::MediaStatus::EndOfMedia);
			return;
		}
			// play next url
		auto next_index = m_playlist->currentIndex() + 1;

		if (m_playlist->mediaCount() > next_index)
		{
			m_playlist->setCurrentIndex(next_index);
			auto url = m_playlist->currentMedia().canonicalUrl().toString().toStdString();

			play(url);
		}
	}, Qt::QueuedConnection);
}

FFPlayer::~FFPlayer()
{
	d_ptr->avsync.stop();
	d_ptr->vdecoder.stop();
	if (d_ptr->demuxer)
		d_ptr->demuxer->stop();
	demux_thread.quit();

	demux_thread.wait();

    delete d_ptr;
}

static int stream_index(enum AVMediaType type, AVFormatContext *ctx)
{
	unsigned int i;

	for (i = 0; (unsigned int) i < ctx->nb_streams; i++)
		if (ctx->streams[i]->codec->codec_type == type)
			return i;
	return -1;
}

void FFPlayer::load(std::string url)
{
	d_ptr->avsync.stop();
	d_ptr->adecoder.close_codec();
	d_ptr->vdecoder.close_codec();

	{
	QMutexLocker l(&(d_func()->m_repload_mutex));

	if ( d_ptr->avformat_ctx_preloading_next)
	{
		d_ptr->avformat_ctx = std::move(d_ptr->avformat_ctx_preloading_next);

		qDebug() << "using preloaded flv";
	}else
	{
		AVFormatContext* avformat_ctx = nullptr;//avformat_alloc_context();

		mediaStatusChanged(QMediaPlayer::MediaStatus::LoadingMedia);

		avformat_open_input(&avformat_ctx, url.c_str(), NULL, NULL);

		d_func()->avformat_ctx.reset(avformat_ctx, avformat_free_context);

		avformat_find_stream_info(avformat_ctx, NULL);
	}
	}

	mediaStatusChanged(QMediaPlayer::MediaStatus::LoadedMedia);

	Q_EMIT durationChanged(d_ptr->avformat_ctx->duration * 1000 / AV_TIME_BASE);

	// 获取 video index 和 audio index

	int video_index, audio_index;

	video_index = stream_index(AVMEDIA_TYPE_VIDEO, d_ptr->avformat_ctx.get());
	audio_index = stream_index(AVMEDIA_TYPE_AUDIO, d_ptr->avformat_ctx.get());

	// 开启线程，正式进入播放

	delete d_func()->demuxer;
	d_func()->demuxer = new QDemuxer(this);
	d_func()->demuxer->moveToThread(&demux_thread);

	// 创建 demux 对象.
	auto astream = d_ptr->avformat_ctx->streams[audio_index];
	auto vstream = d_ptr->avformat_ctx->streams[video_index];

	d_ptr->adecoder.init_decoder(astream, audio_index);
	d_ptr->vdecoder.init_decoder(vstream, video_index);

	connect(d_func()->demuxer, SIGNAL(frame_readed(AVPacket*)), &d_ptr->vdecoder, SLOT(put_one_frame(AVPacket*)));
	connect(d_func()->demuxer, SIGNAL(frame_readed(AVPacket*)), &d_ptr->adecoder, SLOT(put_one_frame(AVPacket*)));

	connect(&d_ptr->avsync, SIGNAL(need_more_frame()), d_func()->demuxer, SLOT(slot_start()), Qt::QueuedConnection);

	connect(d_ptr->demuxer, SIGNAL(frame_seeked()), &d_ptr->avsync, SLOT(frame_seeked()));

	d_func()->demuxer->start();

	// delete the demuxer
	connect(d_ptr->demuxer, SIGNAL(frame_done()), &d_ptr->avsync, SLOT(slot_frame_done()), Qt::QueuedConnection);
	connect(d_ptr->demuxer, SIGNAL(frame_done()), d_ptr->demuxer, SLOT(deleteLater()));

	connect(d_ptr->demuxer, &QDemuxer::frame_done, this, &FFPlayer::preload_next_uri, Qt::DirectConnection);

	m_state = QMediaPlayer::PausedState;
}

void FFPlayer::preload_next_uri()
{
	QMutexLocker l(&(d_func()->m_repload_mutex));

	if (d_func()->avformat_ctx_preloading_next)
		return;

	if ( m_state != QMediaPlayer::PlayingState)
	{
		return;
	}

	AVFormatContext* avformat_ctx = nullptr;//avformat_alloc_context();

	mediaStatusChanged(QMediaPlayer::MediaStatus::LoadingMedia);

	auto i = m_playlist->nextIndex();

	if ( i == -1 )
		return;

	auto url = m_playlist->media(i).canonicalUrl().toString();

	qDebug() << "preloading " << url;

	avformat_open_input(&avformat_ctx, url.toStdString().c_str(), NULL, NULL);

	avformat_find_stream_info(avformat_ctx, NULL);
	d_func()->avformat_ctx_preloading_next.reset(avformat_ctx, avformat_free_context);
	av_dump_format(d_ptr->avformat_ctx.get(), 0, NULL, 0);
}

void FFPlayer::play(std::string url)
{
	load(url);
	setProperty("MediaStatus", QMediaPlayer::MediaStatus::BufferingMedia);
	mediaStatusChanged(QMediaPlayer::MediaStatus::BufferingMedia);
	d_ptr->avsync.start();
}

void FFPlayer::stop()
{
	m_state = QMediaPlayer::StoppedState;
	if (d_ptr->demuxer)
		d_ptr->demuxer->stop();
	d_ptr->avsync.stop();
}

void FFPlayer::pause()
{
	if (m_state != QMediaPlayer::PlayingState)
		return;
	// 进入暂停功能
	d_func()->avsync.pause();
}

void FFPlayer::setPosition(qint64 position)
{
	if (!d_func()->demuxer)
		return;

	// 忽略之
	if (m_state == QMediaPlayer::StoppedState)
		return;

	// 先停止播放.
	d_func()->avsync.stop();

	// 接着快进
	d_func()->demuxer->setPosition(position);
	d_func()->avsync.clear_queue();

	d_func()->avsync.start();
}

QMediaPlayer::MediaStatus FFPlayer::MediaStatus() const
{
	return m_MediaStatus;
}


void FFPlayer::render_frame(const QVideoFrame&f)
{
	if (m_current_frame_size != f.size())
	{
		m_current_frame_size = f.size();
		metaDataChanged("Resolution", f.size());
	}

	positionChanged(f.startTime());

	if (m_vout)
		m_vout->present(f);
}

void FFPlayer::load()
{
	// 从 play list 里获取url

	auto i = m_playlist->currentIndex();

	if ( i == -1)
		m_playlist->setCurrentIndex(0);

	auto url = m_playlist->currentMedia().canonicalUrl().toString().toStdString();

	load(url);
}

void FFPlayer::play()
{
	if ( m_state == QMediaPlayer::StoppedState)
	{
		if (m_MediaStatus == QMediaPlayer::LoadedMedia)
		{
			setProperty("MediaStatus", QMediaPlayer::MediaStatus::BufferingMedia);
			mediaStatusChanged(QMediaPlayer::MediaStatus::BufferingMedia);
			d_ptr->avsync.start();
			return;
		}

		// 从 play list 里获取url

		auto i = m_playlist->currentIndex();

		if ( i == -1)
			m_playlist->setCurrentIndex(0);

		auto url = m_playlist->currentMedia().canonicalUrl().toString().toStdString();

		play(url);

	}else if (m_state == QMediaPlayer::PausedState)
	{
		d_func()->avsync.resume();
	}
}

void FFPlayer::start_decode(QIODevice* input)
{
	auto avio =  QIO2AVIO(input);

	auto avformat_ctx = avformat_alloc_context();

	AVInputFormat* fmt = nullptr;

	av_probe_input_buffer(avio.avio_context(), &fmt, "", 0, 0, 0);

	avformat_open_input(&avformat_ctx, NULL, fmt, NULL);

	avformat_find_stream_info(avformat_ctx, NULL);

	av_dump_format(avformat_ctx, 0, NULL, 0);
}
