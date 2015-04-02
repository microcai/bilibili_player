
#include <QtCore>

extern "C"
{

#include <libavcodec/vdpau.h>
#include <libavcodec/vaapi.h>

}

#include "ffplayer.hpp"
#include "ffplayer_p.hpp"
#include "qio2avio.hpp"

FFPlayerPrivate::FFPlayerPrivate(FFPlayer* q)
	: q_ptr(q)
{
	avcodec_register_all();
	av_register_all();
}

FFPlayerPrivate::~FFPlayerPrivate()
{
}

FFPlayer::FFPlayer()
 : d_ptr(new FFPlayerPrivate(this))
{

}

FFPlayer::~FFPlayer()
{
    delete d_ptr;
}

void FFPlayer::play(std::string url)
{
	auto avformat_ctx = avformat_alloc_context();

	avformat_open_input(&avformat_ctx, NULL, NULL, NULL);

	avformat_find_stream_info(avformat_ctx, NULL);

	av_dump_format(avformat_ctx, 0, NULL, 0);

// 	avpk
	AVPacket pkt;
	av_read_frame(avformat_ctx,&pkt);

// 	avcodec_decode_video2(avformat_ctx, &pkt, );
}


void FFPlayer::play()
{
	// 从 play list 里获取url

	auto url = m_playlist->currentMedia().canonicalUrl().toString().toStdString();


	play(url);

	// 打开 url

	// TODO
	// 缓冲到一定的数据量

	// 开始播放

	// 播放过程中监控剩余时间，剩余时间剩下 30s 的时候开始缓冲下一个
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

// 	avpk
	AVPacket pkt;
	av_read_frame(avformat_ctx,&pkt);

// 	avcodec_decode_video2(avformat_ctx, &pkt, );
}
