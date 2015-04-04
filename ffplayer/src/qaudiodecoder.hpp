
#pragma once

#include <memory>
#include <QtCore>
#include <QAudioBuffer>
#include "ffplayer.hpp"
#include "ffmpeg.hpp"

class QADecoder : public QObject
{
	Q_OBJECT
public:
	QADecoder(FFPlayer* parent);

	// emit to vidersync
	Q_SIGNAL void audioframe_decoded(const QAudioBuffer&);

	virtual ~QADecoder();

	void stop();

	Q_SLOT void init_decoder(AVStream* audio_strem, int audio_index);

private:
	// called by demuxer
	Q_SLOT void put_one_frame(AVPacket*);

	Q_SLOT void decode_one_frame(std::shared_ptr<AVPacket>);
	Q_SLOT void avframe_decoded(std::shared_ptr<AVFrame>);
	void close_codec();

private:
	std::shared_ptr<AVFrame> current_audio_frame;

	bool m_stop = false;

	FFPlayer* parent;

	int audio_index;
	AVCodecID codec_id;

	AVCodec* codec;

	AVCodecContext* codec_context;
	AVStream* audiostream;
};
