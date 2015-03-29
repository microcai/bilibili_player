
#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <QtCore>



class FFPlayer;
class FFPlayerPrivate
{
public:
    FFPlayerPrivate(FFPlayer* q);
    virtual ~FFPlayerPrivate();

private:
    FFPlayer* const q_ptr;
    Q_DECLARE_PUBLIC(FFPlayer)
};

