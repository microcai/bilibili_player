
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
#include <QGraphicsScene>

#include <QSlider>
#include <QDomDocument>

#include "videoitem.hpp"
#include "qgraphicsbusybufferingitem.hpp"
#include "graphicssvgitem.hpp"

#include "compositionsuspender.hpp"
#include "screensaver/screensaverinhibitor.hpp"
#include "asssubtitlesitem.hpp"

#include "defs.hpp"
#include "danmumanager.hpp"

#include "player.hpp"

class BPlayer : public Player
{
	Q_OBJECT
	Q_PROPERTY(bool UseBullet MEMBER  use_bullet)
	Q_PROPERTY(double AllowAnySize MEMBER allow_any_resize)
    Q_PROPERTY(double ZoomLevel READ ZoomLevel WRITE SetZoomLevel)
    Q_PROPERTY(QString asspath MEMBER asspath )

public:
	BPlayer(bool use_gl = true);
	virtual ~BPlayer();

	double ZoomLevel() const
	{
		return zoom_level;
	}

	void SetZoomLevel(double v)
	{
		auto s = property("VideoSize").toSizeF() * v;

		if (isFullScreen())
		{
			// also set window Size
			force_video_widget_size(s);
		}else
		{
			resize(s.width(), s.height());
		}
	}

public Q_SLOTS:

	void toggle_full_screen_mode();
	void append_video_url(VideoURL url);

	void set_barrage_dom(QDomDocument barrage);

	void start_play();

private Q_SLOTS:

	void add_barrage(const Moving_Comment& c);
	void slot_mediaChanged(int);

	void play_position_update(qreal);
	void play_position_fast_forwarded(qreal);

	void slot_video_size_changed(QSizeF);

private:
	Moving_Comments m_comments;
	Moving_Comments::const_iterator m_comment_pos;

	double zoom_level = qQNaN();
	int lastY = 0;

	DanmuManager m_danmumgr;
	bool use_bullet = false;
	bool allow_any_resize = false;
	QString asspath;
};
