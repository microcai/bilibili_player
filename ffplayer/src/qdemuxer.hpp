
#pragma once

#include <QtCore>
#include "ffplayer.hpp"
#include "ffmpeg.hpp"

class QDemuxer : public QObject
{
	Q_OBJECT
public:
	QDemuxer(FFPlayer* parent);

	Q_SIGNAL void start();

	// must use Qt::BlockingQueuedConnection to connect!
	Q_SIGNAL void frame_readed(AVPacket*);
	Q_SIGNAL void setPosition(qint64 position);
	Q_SIGNAL void frame_seeked();

public Q_SLOTS:
	void slot_start();
	void stop();

private Q_SLOTS:
	void read_one_frame();
	void read_many_frame();
	void do_setPosition(qint64 position);

private:
	AVPacket pkt;
	FFPlayer* parent;
	bool m_stop = false;
};
