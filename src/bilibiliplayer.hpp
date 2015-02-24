
#pragma once

#include <QObject>
#include <QMainWindow>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>
#include <QGraphicsVideoItem>

#include <QSlider>
#include <QDomDocument>

#include "bilibilidef.hpp"

class BiliBiliPlayer : public QObject
{
	Q_OBJECT

public:
	BiliBiliPlayer();
	virtual ~BiliBiliPlayer();

Q_SIGNALS:
	void full_screen_mode_changed(bool);

public Q_SLOTS:

	void set_full_screen_mode(bool v);

	void append_video_url(BiliBili_VideoURL url);

	void set_barrage_dom(QDomDocument barrage);

	void start_play();

private Q_SLOTS:

	void add_barrage(const BiliBili_Comment& c);

	void drag_slide_done();

	void drag_slide(int p);

	void positionChanged(qint64 position);
	void durationChanged(qint64 duration);

	void slot_metaDataChanged(QString key,QVariant v);

	void slot_mediaChanged(int);
protected:
	std::pair<int, qint64> map_position_to_media(qint64);
	qint64 map_position_from_media(qint64);

private:
	QMainWindow* m_mainwindow = nullptr;
	QGraphicsScene *scene;
	QGraphicsView *graphicsView;

	QMediaPlaylist* play_list;
	QMediaPlayer* vplayer;
	QGraphicsVideoItem* videoItem;
	QSlider * position_slide;

	BiliBili_Comments m_comments;
	BiliBili_Comments::const_iterator m_comment_pos;
	BiliBili_VideoURLs urls;

	QSizeF video_size;
	double zoom_level = 1.0;

	int _drag_positoin = -1;

	bool full_screen_mode = false;
};
