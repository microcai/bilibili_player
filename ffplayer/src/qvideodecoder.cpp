
#include "qvideodecoder.hpp"
#include "ffmpeg.hpp"
#include "ffplayer_p.hpp"

#include <va/va.h>
#include <va/va_x11.h>

#include <QX11Info>

struct hwdec_profile_entry {
    enum AVCodecID av_codec;
    int ff_profile;
    uint64_t hw_profile;
};

// #define PE(av_codec_id, ff_profile, vdp_profile)                \
//     {AV_CODEC_ID_ ## av_codec_id, FF_PROFILE_ ## ff_profile,    \
//      VAProfile ## vdp_profile}
//
// static const hwdec_profile_entry profiles[] = {
//     PE(MPEG2VIDEO,  MPEG2_MAIN,         MPEG2Main),
//     PE(MPEG2VIDEO,  MPEG2_SIMPLE,       MPEG2Simple),
//     PE(MPEG4,       MPEG4_ADVANCED_SIMPLE, MPEG4AdvancedSimple),
//     PE(MPEG4,       MPEG4_MAIN,         MPEG4Main),
//     PE(MPEG4,       MPEG4_SIMPLE,       MPEG4Simple),
//     PE(H264,        H264_HIGH,          H264High),
//     PE(H264,        H264_MAIN,          H264Main),
//     PE(H264,        H264_BASELINE,      H264Baseline),
//     PE(VC1,         VC1_ADVANCED,       VC1Advanced),
//     PE(VC1,         VC1_MAIN,           VC1Main),
//     PE(VC1,         VC1_SIMPLE,         VC1Simple),
//     PE(WMV3,        VC1_ADVANCED,       VC1Advanced),
//     PE(WMV3,        VC1_MAIN,           VC1Main),current_packet
//     PE(WMV3,        VC1_SIMPLE,         VC1Simple),
//     {0}
// };
//


struct QAVframeBuffer : public QAbstractPlanarVideoBuffer
{
	virtual int map(MapMode mode, int* numBytes, int bytesPerLine[4], uchar* data[4])
	{
		*numBytes = std::accumulate(std::begin(frame->linesize), std::end(frame->linesize),0);

		for(int i=0; i<4;i++)
		{
			bytesPerLine[i] = frame->linesize[i];
			data[i] = frame->data[i];
		}

		return 3;
	}

	virtual ~QAVframeBuffer()
	{
		unmap();
	}

	virtual void unmap(){}

	virtual MapMode mapMode() const
	{
		return ReadWrite;
	}

	virtual void release()
	{
		av_frame_free(&frame);
		delete this;
	}

	// take ownership
	QAVframeBuffer(AVFrame* _f)
		 : QAbstractPlanarVideoBuffer(QAbstractVideoBuffer::NoHandle)
	{
		frame = _f;
	}

	AVFrame* frame;
};

void QVDecoder::decode_one_frame(std::shared_ptr<AVPacket> current_packet)
{
	if(m_stop)
	{
		return;
	}

	int got_picture = 0;

	AVFrame* current_video_frame = av_frame_alloc();

	avcodec_decode_video2(codec_context, current_video_frame, &got_picture, current_packet.get());
	current_video_frame->pts = current_packet->pts;

	if (got_picture)
	{
		// 显示这一 frame 吧？
		frame_decoded(current_video_frame);
	}else
	{
		av_frame_free(&current_video_frame);
	}
}

void QVDecoder::slot_frame_decoded(AVFrame* avframe)
{
	if(m_stop)
		return;
	// 构造 QVideoFrame
	int frame_data_size = 0;

	QSize frame_size(avframe->width, avframe->height);

	QVideoFrame current_frame(new QAVframeBuffer(avframe), frame_size, QVideoFrame::Format_YUV420P);

	current_frame.setStartTime(avframe->pts * av_q2d(videostream->time_base) * 1000);
	current_frame.setEndTime((avframe->pts + av_frame_get_pkt_duration(avframe)) * av_q2d(videostream->time_base) * 1000);

	videoframe_decoded(current_frame);
}

void QVDecoder::put_one_frame(AVPacket* src)
{
	if(m_stop)
		return;

	if ( src->stream_index == video_index)
	{
		std::shared_ptr<AVPacket> current_packet(new AVPacket, [](AVPacket*p){
			av_free_packet(p);
			delete p;
		});
		av_init_packet(current_packet.get());
		av_copy_packet(current_packet.get(), src);

 		do_decode_one_frame(current_packet);
	}
	return;
}

void QVDecoder::stop()
{
	m_stop = true;
}

QVDecoder::QVDecoder(FFPlayer* _parent, int _video_index)
	: video_index(_video_index)
	, parent(_parent)
{
	av_init_packet(&current_packet);

	connect(this, SIGNAL(frame_decoded(AVFrame*)), this, SLOT(slot_frame_decoded(AVFrame*)));
	connect(this, SIGNAL(do_decode_one_frame(std::shared_ptr<AVPacket>)), this, SLOT(decode_one_frame(std::shared_ptr<AVPacket>)));

	videostream = parent->d_ptr->avformat_ctx.get()->streams[video_index];

	codec_context = videostream->codec;

	codec_id = codec_context->codec_id;

	if (0) // (codec_id == AV_CODEC_ID_H264)
	{

		open_hw_accel_codec(codec_id);

	}else{
		codec = avcodec_find_decoder(codec_id);
		avcodec_open2(codec_context, codec, NULL);
	}
}

QVDecoder::~QVDecoder()
{
	avcodec_close(codec_context);
}

void QVDecoder::open_hw_accel_codec(AVCodecID codec_id)
{
	// 根据系统平台选择解码器
	parent->d_ptr->avformat_ctx;

	auto avformat_ctx = parent->d_ptr->avformat_ctx.get();

	codec = avcodec_find_decoder_by_name("h264_vdpau");

	avcodec_open2(codec_context, codec, NULL);

// // 	m_va_context.display = v

	Display* x11display = QX11Info::display();

	m_va_context.display = vaGetDisplay(x11display);

// 	VAProfileH264High;
// 	vaCreateConfig();
//

	codec_context->hwaccel_context = &m_va_context;
}
