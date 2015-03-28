
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
	Q_PROPERTY(QString VideoAspect MEMBER  VideoAspect)
    Q_PROPERTY(double ZoomLevel READ ZoomLevel WRITE SetZoomLevel NOTIFY ZoomLevelChanged)
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
		if(zoom_level !=v)
		{
			zoom_level = v;
			ZoomLevelChanged(v);
		}
	}

Q_SIGNALS:
	void ZoomLevelChanged(double);

public Q_SLOTS:

	void toogle_full_screen_mode();
	void append_video_url(VideoURL url);

	void set_barrage_dom(QDomDocument barrage);

	void start_play();



private Q_SLOTS:

	void add_barrage(const Moving_Comment& c);


	void slot_metaDataChanged(QString key,QVariant v);

	void slot_mediaChanged(int);

protected:
	std::pair<int, qint64> map_position_to_media(qint64);
	qint64 map_position_from_media(qint64);

private:
	Moving_Comments m_comments;
	Moving_Comments::const_iterator m_comment_pos;

	double zoom_level = qQNaN();
	int lastY = 0;

	DanmuManager m_danmumgr;
	bool use_bullet = false;
	QString VideoAspect = "auto";
	bool allow_any_resize = false;
	QString asspath;
};
