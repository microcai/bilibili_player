
#pragma once

#include <QtCore>
#include <memory>
#include "ffmpeg.hpp"

#include "qdemuxer.hpp"
#include "qvideodecoder.hpp"

class FFPlayer;
class FFPlayerPrivate
{
public:
    FFPlayerPrivate(FFPlayer* q);
    virtual ~FFPlayerPrivate();

    FFPlayer* const q_ptr;
    Q_DECLARE_PUBLIC(FFPlayer)


	std::shared_ptr<AVIOContext> avio_ctx;

	std::shared_ptr<AVFormatContext> avformat_ctx;

protected:
	// size of the buffer! 64MB for now
	// should be enough for smooth playback
	char buffer[1024*1024*64];
private:

	QDemuxer* demuxer;
	QVDecoder* decoder;
};
