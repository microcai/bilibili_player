
#pragma once

#include <QtCore>
#include <QVideoFrame>
#include "ffplayer.hpp"
#include "ffplayer_p.hpp"

class QVDecoder : public QObject
{
	Q_OBJECT
public:
	QVDecoder(FFPlayer* parent, int video_index);

	Q_SIGNAL void videoframe_decoded(const QVideoFrame&);

	virtual ~QVDecoder();

	Q_SLOT void put_one_frame(AVPacket*);

	int video_index;
	AVCodecID codec_id;

	AVCodec* codec;

	AVCodecContext* codec_context;
	AVStream* videostream;

protected:

	void open_hw_accel_codec(AVCodecID codec_id);

	FFPlayer* parent;

	vaapi_context m_va_context;

private:
	Q_SLOT void decode_one_frame();
	Q_SIGNAL void frame_decoded(AVFrame*);
	Q_SLOT void slot_frame_decoded(AVFrame*);

	AVPacket current_packet;
};
