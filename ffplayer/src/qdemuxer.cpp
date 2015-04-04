
#include "qdemuxer.hpp"

#include "ffmpeg.hpp"
#include "ffplayer_p.hpp"

Q_DECLARE_OPAQUE_POINTER(AVPacket*);

QDemuxer::QDemuxer(FFPlayer* _parent)
	: parent(_parent)
{
	connect(this, SIGNAL(start()), this, SLOT(slot_start()), Qt::QueuedConnection);
	connect(this, SIGNAL(setPosition(qint64)), this, SLOT(do_setPosition(qint64)), Qt::QueuedConnection);

	qRegisterMetaType<AVPacket*>("AVPacket*");
}

void QDemuxer::stop()
{
	m_stop = true;
}


void QDemuxer::slot_start()
{
	// 开始解码
	QTimer::singleShot(0, this, SLOT(read_many_frame()));

	pkt.pts = 0;
}

void QDemuxer::read_many_frame()
{
	for(int i = 0; i < 10; i++)
	{
		read_one_frame();
	}
}

void QDemuxer::read_one_frame()
{
	if (m_stop)
		return;

	auto avformat_ctx = parent->d_ptr->avformat_ctx.get();

	av_init_packet(&pkt);
	if (av_read_frame(avformat_ctx, &pkt) >=0)
	{
		av_dup_packet(&pkt);
		frame_readed(&pkt);
	}
	else
	{
		// EOF
	}

	av_free_packet(&pkt);
}

void QDemuxer::do_setPosition(qint64 position)
{
	auto avformat_ctx = parent->d_ptr->avformat_ctx.get();

	int64_t seek_target = 0;// position * AV_TIME_BASE;
	int64_t seek_min    = /*play->m_seek_rel > 0 ? seek_target - play->m_seek_rel + 2:*/ INT64_MIN;
	int64_t seek_max    = /*play->m_seek_rel < 0 ? seek_target - play->m_seek_rel - 2:*/ INT64_MAX;

	int seek_flags = 0 & (~AVSEEK_FLAG_BYTE);

	auto ret = avformat_seek_file(avformat_ctx, -1, seek_min, seek_target, seek_max, seek_flags);

	if (ret <0)
	{
		qDebug() <<  "seek failed";
	}

	frame_seeked();
}

