
#include <QtCore>

extern "C"
{

#include <libavcodec/vdpau.h>
#include <libavcodec/vaapi.h>

}

#include "ffplayer.hpp"
#include "ffplayer_p.hpp"

FFPlayerPrivate::FFPlayerPrivate(FFPlayer* q)
	: q_ptr(q)
{
	avcodec_register_all();
	av_register_all();



}



FFPlayerPrivate::~FFPlayerPrivate()
{
}


FFPlayer::FFPlayer()
 : d_ptr(new FFPlayerPrivate(this))
{

}

FFPlayer::~FFPlayer()
{
    delete d_ptr;
}


void FFPlayer::start_decode(QIODevice* input)
{
// 	avformat_open_input();

}
