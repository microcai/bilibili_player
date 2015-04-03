
#include "qdemuxer.hpp"

#include "ffmpeg.hpp"
#include "ffplayer_p.hpp"

Q_DECLARE_OPAQUE_POINTER(AVPacket*);

QDemuxer::QDemuxer(FFPlayer* _parent)
	: parent(_parent)
{
	connect(this, SIGNAL(start()), this, SLOT(slot_start()));
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
