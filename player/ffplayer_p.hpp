/*
 * 
 */

#ifndef FFPLAYER_PRIVATE_H
#define FFPLAYER_PRIVATE_H

#include <QtCore/qglobal.h>

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

#endif // FFPLAYER_PRIVATE_H
