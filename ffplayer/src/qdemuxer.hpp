
#pragma once

#include <QtCore>
#include "ffplayer.hpp"
#include "ffplayer_p.hpp"

class QDemuxer : public QObject
{
	Q_OBJECT
public:
	QDemuxer(FFPlayer* parent);

	Q_SIGNAL void start();

	// must use Qt::BlockingQueuedConnection to connect!
	Q_SIGNAL void frame_readed(AVPacket*);

public Q_SLOTS:
	void slot_start();



private:
	AVPacket pkt;

	FFPlayer* parent;
	Q_SLOT void read_one_frame();
};
