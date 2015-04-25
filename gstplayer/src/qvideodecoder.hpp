
#pragma once

#include <memory>
#include <QtCore>
#include <QVideoFrame>
#include "gstplayer.hpp"
#include "ffmpeg.hpp"

class QVDecoder : public QObject
{
	Q_OBJECT
public:
	QVDecoder(FFPlayer* parent);

	// emit to vidersync
	Q_SIGNAL void videoframe_decoded(const QVideoFrame&);

	virtual ~QVDecoder();

	Q_SLOT void init_decoder(AVStream*, int video_index);
	Q_SLOT void close_codec();

	void stop();

private:
	// called by demuxer
	Q_SLOT void put_one_frame(AVPacket*);

	Q_SLOT void decode_one_frame(std::shared_ptr<AVPacket>);

	Q_SLOT void avframe_decoded(std::shared_ptr<AVFrame>);

private:
	void open_hw_accel_codec(AVCodecID codec_id);

private:
	std::shared_ptr<AVFrame> current_video_frame;


	bool m_stop = false;

	FFPlayer* parent;

	vaapi_context m_va_context;

	int video_index;
	AVCodecID codec_id;

	AVCodec* codec;

	AVCodecContext* codec_context;
	AVStream* videostream;
};
