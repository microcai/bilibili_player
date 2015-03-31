
#pragma once

#include <QtCore>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <memory>


class FFPlayer;
class FFPlayerPrivate
{
public:
    FFPlayerPrivate(FFPlayer* q);
    virtual ~FFPlayerPrivate();

private:
    FFPlayer* const q_ptr;
    Q_DECLARE_PUBLIC(FFPlayer)


	std::shared_ptr<AVIOContext> avio_ctx;

protected:
	// size of the buffer! 64MB for now
	// should be enough for smooth playback
	char buffer[1024*1024*64];
private:

};
