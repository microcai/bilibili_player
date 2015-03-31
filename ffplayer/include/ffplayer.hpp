#ifndef FFPLAYER_H
#define FFPLAYER_H

#include <QtCore>

class FFPlayerPrivate;

class FFPlayer : public QObject
{
    Q_OBJECT
public:
	FFPlayer();
	~FFPlayer();

	// FFPlayer take owner ship of input
	void start_decode(QIODevice* input);

private:
    class FFPlayerPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(FFPlayer)
};

#endif // FFPLAYER_H
