
#pragma once

#include <memory>

#include <QObject>
#include <QPointer>
#include <QShortcut>
#include <QMainWindow>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>
#include <QGraphicsItem>
#include <QAbstractVideoSurface>
#include <QGraphicsVideoItem>

#include <QSlider>
#include <QDomDocument>

#include "videoitem.hpp"


#include "screensaverinhibitor.hpp"
#include "defs.hpp"
#include "danmumanager.hpp"

class BPlayer : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool UseBullet MEMBER  use_bullet)
	Q_PROPERTY(bool UseOpenGL MEMBER  use_gl)
    Q_PROPERTY(double ZoomLevel READ ZoomLevel WRITE SetZoomLevel NOTIFY ZoomLevelChanged)
    Q_PROPERTY(double full_screen READ full_screen_mode WRITE set_full_screen_mode NOTIFY full_screen_mode_changed)

public:
	BPlayer(QObject * parent = nullptr);
	virtual ~BPlayer();

	double ZoomLevel() const
	{
		return zoom_level;
	}

	void SetZoomLevel(double v)
	{
		if(zoom_level !=v)
		{
			zoom_level = v;
			ZoomLevelChanged(v);
		}
	}

	bool full_screen_mode() const {
		return m_mainwindow->isFullScreen();
	}

Q_SIGNALS:
	void full_screen_mode_changed(bool);
	void ZoomLevelChanged(double);

public Q_SLOTS:

	void toogle_full_screen_mode();
	void set_full_screen_mode(bool v);

	void append_video_url(VideoURL url);

	void set_barrage_dom(QDomDocument barrage);

	void start_play();

	void toogle_play_pause();

	void zoom_in();
	void zoom_out();

private Q_SLOTS:

	void add_barrage(const Moving_Comment& c);

	void drag_slide_done();

	void drag_slide(int p);

	void positionChanged(qint64 position);
	void durationChanged(qint64 duration);

	void slot_metaDataChanged(QString key,QVariant v);

	void slot_mediaChanged(int);

	void adjust_window_size();

	void play_state_changed(QMediaPlayer::State);
    void slot_full_screen_mode_changed(bool);
	void slot_mediaStatusChanged(QMediaPlayer::MediaStatus);

	void fast_forward(); // call this to forward one minite/2

	void fast_backwork(); // call this to back forward one minite/2

protected:
	std::pair<int, qint64> map_position_to_media(qint64);
	qint64 map_position_from_media(qint64);

private:
	QMainWindow* m_mainwindow = nullptr;
	QGraphicsScene *scene;
	QGraphicsView *graphicsView;

	QMediaPlaylist* play_list;
	QMediaPlayer* vplayer;

	QPointer<QGraphicsVideoItem> videoItem;
	QPointer<VideoItem> video_surface;

	QPointer<QGraphicsObject> play_indicator;
	QPointer<QGraphicsObject> pause_indicator;

	QSlider * position_slide;

	Moving_Comments m_comments;
	Moving_Comments::const_iterator m_comment_pos;
	VideoURLs urls;

	QSizeF video_size;
	double zoom_level = qQNaN();
	int lastY = 0;

	int _drag_positoin = -1;

	QScopedPointer<ScreenSaverInhibitor> m_screesave_inhibitor;

	DanmuManager m_danmumgr;
	bool use_bullet = false;
	bool use_gl = false;
};
