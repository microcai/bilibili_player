
#pragma once

#include <QtCore>
#include <memory>
#include "ffmpeg.hpp"

#include <gst/gst.h>


class FFPlayer;
class FFPlayerPrivate
{
public:
    FFPlayerPrivate(FFPlayer* q);
    virtual ~FFPlayerPrivate();

    FFPlayer* const q_ptr;
    Q_DECLARE_PUBLIC(FFPlayer)

protected:
	// size of the buffer! 64MB for now
	// should be enough for smooth playback
private:
	QMutex m_repload_mutex;
	std::shared_ptr<GstPipeline> pipeline;
	GstElement* audio_convert;
	GstElement* audio_sink;
	GstElement* video_sink;
};
