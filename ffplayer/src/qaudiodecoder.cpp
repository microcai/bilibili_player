
#include "qaudiodecoder.hpp"
#include "ffmpeg.hpp"
#include "ffplayer_p.hpp"


void QADecoder::decode_one_frame(std::shared_ptr<AVPacket> current_packet)
{
	if(m_stop)
	{
		return;
	}

	int got_picture = 0;


	auto pkt2 = *current_packet;

	while (!m_stop)
	{
		int got_frame = 0;
		auto ret = avcodec_decode_audio4(codec_context, current_audio_frame.get(), &got_frame, &pkt2);
		if (ret < 0)
		{
			printf("Audio error while decoding one frame!!!\n");
			break;
		}
		pkt2.size -= ret;
		pkt2.data += ret;

		/* 不足一个帧, 并且packet中还有数据, 继续解码当前音频packet. */
		if (!got_frame && pkt2.size > 0)
			continue;

		/* packet中已经没有数据了, 并且不足一个帧, 丢弃这个音频packet. */
		if (pkt2.size == 0 && !got_frame)
			break;

		if (current_audio_frame->linesize[0] != 0)
		{
			// 显示这一 frame 吧？

			avframe_decoded(current_audio_frame);

			current_audio_frame.reset(av_frame_alloc(),
				[](AVFrame*current_video_frame){av_frame_free(&current_video_frame);}
			);
			/* packet中数据已经没有数据了, 解码下一个音频packet. */
			if (pkt2.size <= 0)
				break;
		}
	}
}

void QADecoder::avframe_decoded(std::shared_ptr<AVFrame> avframe)
{
	if(m_stop)
		return;
	// 构造 QVideoFrame
	int frame_data_size = 0;
	bool planer = false;

// 	QSize frame_size(avframe->width, avframe->height);

	QAudioFormat format;

	format.setChannelCount(av_frame_get_channels(avframe.get()));
	format.setSampleRate(av_frame_get_sample_rate(avframe.get()));

	switch(avframe->format)
	{
		case AV_SAMPLE_FMT_S16P:
			planer = true;
		case AV_SAMPLE_FMT_S16:
			format.setSampleSize(16);
			format.setSampleType(QAudioFormat::SignedInt);
			break;
		case AV_SAMPLE_FMT_U8P:
			planer = true;
		case AV_SAMPLE_FMT_U8:
			format.setSampleSize(8);
			format.setSampleType(QAudioFormat::UnSignedInt);
			break;

		case AV_SAMPLE_FMT_S32P:
			planer = true;
		case AV_SAMPLE_FMT_S32:
			format.setSampleSize(32);
			format.setSampleType(QAudioFormat::SignedInt);
			break;
		case AV_SAMPLE_FMT_FLTP:
			planer = true;
		case AV_SAMPLE_FMT_FLT:
			format.setSampleSize(32);
			format.setSampleType(QAudioFormat::Float);
			break;
		case AV_SAMPLE_FMT_DBLP:
			planer = true;
		case AV_SAMPLE_FMT_DBL:
			format.setSampleSize(64);
			format.setSampleType(QAudioFormat::Float);
			break;
	}

	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setCodec("audio/pcm");

// 	avframe->best_effort_timestamp;
	QAudioBuffer current_frame(avframe->nb_samples, format,
		av_frame_get_best_effort_timestamp(avframe.get()));

	// copy audio
	if (planer)
	{
		auto ptr = current_frame.data<float>();
		auto c = current_frame.byteCount();

		for (int j=0; j < avframe->nb_samples; j++)
		{
			for (int i=0; i < avframe->channels; i++)
			{
				*ptr= reinterpret_cast<float*>(avframe->data[i])[j];
				ptr++;//= avframe->linesize[i];
			}
		}

	}else
	{
		memcpy(current_frame.data(), avframe->data[0], avframe->linesize[0]);
	}

	avframe.reset();
	audioframe_decoded(current_frame);
}

void QADecoder::put_one_frame(AVPacket* src)
{
	if(m_stop)
		return;

	if ( src->stream_index == audio_index)
	{
		std::shared_ptr<AVPacket> current_packet(src, [](AVPacket*p){
// 			av_free_packet(p);
// 			delete p;
		});
// 		av_init_packet(current_packet.get());
// 		av_copy_packet(current_packet.get(), src);


 		decode_one_frame(current_packet);
	}
	return;
}

void QADecoder::stop()
{
	m_stop = true;
}

QADecoder::QADecoder(FFPlayer* _parent)
	: parent(_parent)
{
	codec = nullptr;
	codec_context = nullptr;
}

void QADecoder::init_decoder(AVStream* audio_strem, int audio_index)
{
	close_codec();

	current_audio_frame.reset(av_frame_alloc(),
		[](AVFrame*current_video_frame){av_frame_free(&current_video_frame);}
	);

	audiostream = parent->d_ptr->avformat_ctx.get()->streams[audio_index];

	codec_context = audiostream->codec;

	codec_id = codec_context->codec_id;

	codec = avcodec_find_decoder(codec_id);
	avcodec_open2(codec_context, codec, NULL);
}

void QADecoder::close_codec()
{
	if (codec_context && codec)
	{
		avcodec_close(codec_context);
		codec_context = nullptr;
		codec = nullptr;
	}
}

QADecoder::~QADecoder()
{
	close_codec();
}
