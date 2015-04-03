
#pragma once

#include <QtCore>
#include <QAbstractVideoSurface>
#include <QMediaPlaylist>
#include <QMediaPlayer>
#include <QGraphicsItem>
#include <QGraphicsVideoItem>

class FFPlayerPrivate;
class QDemuxer;
class QVDecoder;
class QAudioVideoSync;
class QADecoder;

class FFPlayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QMediaPlayer::MediaStatus MediaStatus READ MediaStatus WRITE SetMediaStatus NOTIFY mediaStatusChanged)

public:
	FFPlayer();
	~FFPlayer();

	// FFPlayer take owner ship of input
	void start_decode(QIODevice* input);
	void setVideoOutput(QAbstractVideoSurface* v){
		m_vout = v;
	}

	void setVideoOutput(QGraphicsVideoItem* v){
		m_vout2 = v;
	}

	Q_SLOT void play();
	void stop(){}
	void pause();

	QMediaPlaylist * playlist() const {return const_cast<QMediaPlaylist*>(m_playlist);}
	void setPlaylist(QMediaPlaylist * playlist){ m_playlist = playlist;};

	void setPosition(qint64 position){}
	void play(std::string url);

	void setNotifyInterval(int milliSeconds){}

	QMediaPlayer::State state() const { return m_state;}
	QMediaPlayer::MediaStatus MediaStatus() const;
Q_SIGNALS:
	void metaDataChanged(const QString & key, const QVariant & value);
	void mediaStatusChanged(QMediaPlayer::MediaStatus status);
	void durationChanged(qint64 duration);
	void positionChanged(qint64 position);
	void stateChanged(QMediaPlayer::State state);

private:
	void SetMediaStatus(QMediaPlayer::MediaStatus status){m_MediaStatus = status;};

	Q_SLOT void render_frame(const QVideoFrame&);

private:
    class FFPlayerPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(FFPlayer)
	friend class QDemuxer;
	friend class QVDecoder;
	friend class QADecoder;
	friend class QAudioVideoSync;

private:

	QMediaPlayer::MediaStatus m_MediaStatus = QMediaPlayer::UnknownMediaStatus;
	QMediaPlayer::State m_state = QMediaPlayer::StoppedState;

	QMediaPlaylist* m_playlist;

	// 这些个线程用来读取和解码
	QThread demux_thread;

	QAbstractVideoSurface* m_vout = 0;
	QGraphicsVideoItem* m_vout2 = 0;
	QSize m_current_frame_size;
};

