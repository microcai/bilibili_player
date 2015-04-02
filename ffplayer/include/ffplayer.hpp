
#pragma once

#include <QtCore>
#include <QAbstractVideoSurface>
#include <QMediaPlaylist>
#include <QMediaPlayer>
#include <QGraphicsItem>
#include <QGraphicsVideoItem>

class FFPlayerPrivate;

class FFPlayer : public QObject
{
    Q_OBJECT
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
	void pause(){}
	QMediaPlaylist * playlist() const {return const_cast<QMediaPlaylist*>(m_playlist);}
	void setPlaylist(QMediaPlaylist * playlist){ m_playlist = playlist;};

	void setPosition(qint64 position){}
	void play(std::string url);

	void setNotifyInterval(int milliSeconds){}

	QMediaPlayer::State state() const {}

    class FFPlayerPrivate* const d_ptr;
private:

	Q_SLOT void render_frame(const QVideoFrame&);
	Q_SLOT void sync_frame(const QVideoFrame&);

private:
    Q_DECLARE_PRIVATE(FFPlayer)

	QMediaPlaylist* m_playlist;

	// 这些个线程并不是用来渲染视频，而是用来执行音视频同步.
	QThread video_clocked_presenter_thread;
	QThread audio_clocked_presenter_thread;

	// 这些个线程用来解码
	QThread video_decode_thread;
	QThread audio_decode_thread;

	// 这些个线程用来读取
	QThread demux_thread;

	QAbstractVideoSurface* m_vout = 0;
	QGraphicsVideoItem* m_vout2 = 0;

	QTimer m_av_sync_clock;
	QTime m_start_time;
};

