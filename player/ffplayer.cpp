/*
 * 
 */

#include "ffplayer.hpp"
#include "ffplayer_p.hpp"

FFPlayerPrivate::FFPlayerPrivate(FFPlayer* q) : q_ptr(q)
{
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
