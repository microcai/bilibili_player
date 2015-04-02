
#include "qdemuxer.hpp"

#include "ffmpeg.hpp"


Q_DECLARE_OPAQUE_POINTER(AVPacket*);

QDemuxer::QDemuxer(FFPlayer* _parent)
	: parent(_parent)
{
	connect(this, SIGNAL(start()), this, SLOT(slot_start()));
	qRegisterMetaType<AVPacket*>("AVPacket*");
}

void QDemuxer::slot_start()
{
	// 开始解码
	QTimer::singleShot(0, this, SLOT(read_one_frame()));

	pkt.pts = 0;
}

void QDemuxer::read_one_frame()
{

	auto avformat_ctx = parent->d_ptr->avformat_ctx.get();

	av_init_packet(&pkt);
	av_read_frame(avformat_ctx, &pkt);

	QTimer::singleShot(0, this, SLOT(read_one_frame()));

	frame_readed(&pkt);
}
