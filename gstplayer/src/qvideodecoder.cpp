
#include <QOpenGLFunctions>

#include "qvideodecoder.hpp"
#include "ffmpeg.hpp"
#include "ffplayer_p.hpp"

#include <QX11Info>

struct QAVframeBuffer : public QAbstractPlanarVideoBuffer
{
	virtual int map(MapMode mode, int* numBytes, int bytesPerLine[4], uchar* data[4])
	{

		*numBytes = 0;

		for(int i=0; i<3;i++)
		{
			bytesPerLine[i] = frame->linesize[i];
			data[i] = frame->data[i];

			*numBytes += frame->linesize[i] * frame->height;
		}
		return 3;
	}

	virtual ~QAVframeBuffer()
	{
	}

	virtual void unmap(){}

	virtual MapMode mapMode() const
	{
		return ReadWrite;
	}

	virtual void release()
	{
		delete this;
	}

	// take ownership
	QAVframeBuffer(std::shared_ptr<AVFrame> _f)
		 : QAbstractPlanarVideoBuffer(QAbstractVideoBuffer::NoHandle)
	{
		frame = _f;
	}

	std::shared_ptr<AVFrame> frame;
};

void QVDecoder::decode_one_frame(std::shared_ptr<AVPacket> current_packet)
{
	if(m_stop)
	{
		return;
	}

	int got_picture = 0;


	auto pkt2 = *current_packet;

	while (pkt2.size > 0 && !m_stop)
	{
		auto ret = avcodec_decode_video2(codec_context, current_video_frame.get(), &got_picture, &pkt2);
		if (ret < 0)
		{
			printf("Video error while decoding one frame!!!\n");
			break;
		}
		if (got_picture)
			break;
		pkt2.size -= ret;
		pkt2.data += ret;
	}

// 	current_video_frame->pts = current_packet->pts;

	if (got_picture)
	{
		// 显示这一 frame 吧？

		std::shared_ptr<AVFrame> new_frame(av_frame_clone(current_video_frame.get()),
			[](AVFrame*current_video_frame){av_frame_free(&current_video_frame);}
		);

		avframe_decoded(new_frame);

		current_video_frame.reset(av_frame_alloc(),
			[](AVFrame*current_video_frame){av_frame_free(&current_video_frame);}
		);
	}
}

void QVDecoder::avframe_decoded(std::shared_ptr<AVFrame> avframe)
{
	if(m_stop)
		return;
	// 构造 QVideoFrame
	int frame_data_size = 0;

	QSize frame_size(avframe->width, avframe->height);

	QVideoFrame current_frame(new QAVframeBuffer(avframe), frame_size, QVideoFrame::Format_YUV420P);

	current_frame.setStartTime(av_frame_get_best_effort_timestamp(avframe.get()) * av_q2d(videostream->time_base) * 1000);
	current_frame.setEndTime((av_frame_get_best_effort_timestamp(avframe.get()) + av_frame_get_pkt_duration(avframe.get())) * av_q2d(videostream->time_base) * 1000);

	avframe.reset();
	videoframe_decoded(current_frame);
}

void QVDecoder::put_one_frame(AVPacket* src)
{
	if(m_stop)
		return;

	if ( src->stream_index == video_index)
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

void QVDecoder::stop()
{
	m_stop = true;
}

QVDecoder::QVDecoder(FFPlayer* _parent)
	: parent(_parent)
{
	codec = nullptr;
	codec_context = nullptr;
}

void QVDecoder::init_decoder(AVStream* video_stream, int _video_index)
{
	video_index = _video_index;

	close_codec();

	current_video_frame.reset(av_frame_alloc(),
		[](AVFrame*current_video_frame){av_frame_free(&current_video_frame);}
	);

	videostream = video_stream;

	codec_context = videostream->codec;

	codec_id = codec_context->codec_id;

	if (codec_id == AV_CODEC_ID_H264)
	{

		open_hw_accel_codec(codec_id);

	}else{
		codec = avcodec_find_decoder(codec_id);
		avcodec_open2(codec_context, codec, NULL);
	}
}

void QVDecoder::close_codec()
{
	if (codec_context && codec)
	{
		avcodec_close(codec_context);
		codec_context = nullptr;
		codec = nullptr;
	}
}

QVDecoder::~QVDecoder()
{
	close_codec();
}

#ifdef  HAVE_VAAPI

#include <va/va.h>
#include <va/va_glx.h>

struct hwdec_profile_entry {
    enum AVCodecID av_codec;
    int ff_profile;
    uint64_t hw_profile;
};

#define PE(av_codec_id, ff_profile, vdp_profile)                \
    { (AVCodecID) AV_CODEC_ID_ ## av_codec_id, FF_PROFILE_ ## ff_profile,    \
     VAProfile ## vdp_profile}

static const hwdec_profile_entry profiles[] = {
    PE(MPEG2VIDEO,  MPEG2_MAIN,         MPEG2Main),
    PE(MPEG2VIDEO,  MPEG2_SIMPLE,       MPEG2Simple),
    PE(MPEG4,       MPEG4_ADVANCED_SIMPLE, MPEG4AdvancedSimple),
    PE(MPEG4,       MPEG4_MAIN,         MPEG4Main),
    PE(MPEG4,       MPEG4_SIMPLE,       MPEG4Simple),
    PE(H264,        H264_HIGH,          H264High),
    PE(H264,        H264_MAIN,          H264Main),
    PE(H264,        H264_BASELINE,      H264Baseline),
    PE(VC1,         VC1_ADVANCED,       VC1Advanced),
    PE(VC1,         VC1_MAIN,           VC1Main),
    PE(VC1,         VC1_SIMPLE,         VC1Simple),
    PE(WMV3,        VC1_ADVANCED,       VC1Advanced),
    PE(WMV3,        VC1_MAIN,           VC1Main),
    PE(WMV3,        VC1_SIMPLE,         VC1Simple),
};


void QVDecoder::open_hw_accel_codec(AVCodecID codec_id)
{
	// 根据系统平台选择解码器
	parent->d_ptr->avformat_ctx;

	auto avformat_ctx = parent->d_ptr->avformat_ctx.get();

	codec = avcodec_find_decoder_by_name("h264_vdpau");

	avcodec_open2(codec_context, codec, NULL);

// // 	m_va_context.display = v

	Display* x11display = QX11Info::display();

	auto vadisplay =  vaGetDisplayGLX(x11display);

	m_va_context.display = vadisplay;

	GLuint video_texture;

// 	QOpenGLFunctions glfunc(QOpenGLContext::currentContext());

// 	glfunc.initializeOpenGLFunctions();

// 	glfunc.glGenTextures(1, &video_texture);
// 	void* va_surface = nullptr;

// 	glfunc.glBindTexture(GL_TEXTURE_2D, video_texture);

	VASurfaceID va_surface_id;

	VAContextID va_context_id;


	auto vafmt_from_fffmt = [](AVPixelFormat fmt) -> int
	{
		if (fmt == AV_PIX_FMT_YUV420P)
			return VA_RT_FORMAT_YUV420;
		return VA_RT_FORMAT_RGB32;
	};

	VASurfaceAttrib attrs[10];

// 	vaQueryConfigAttributes(vadisplay, VAProfileH264Main, );

	vaCreateSurfaces(vadisplay, vafmt_from_fffmt(videostream->codec->pix_fmt), videostream->codec->width, videostream->codec->height, &va_surface_id, 1, attrs, 0);

// 	vaCreateSurfaceGLX(vadisplay, GL_TEXTURE_2D, video_texture, &va_surface);


// 	VAProfileH264High;
// 	vaCreateConfig();

	m_va_context.config_id = VAProfileH264Main;


	vaCreateContext(vadisplay, VAProfileH264Main, videostream->codec->width, videostream->codec->height, 0, &va_surface_id, 1, & va_context_id );

	m_va_context.context_id = va_context_id;


	codec_context->hwaccel_context = &m_va_context;
}

#endif
