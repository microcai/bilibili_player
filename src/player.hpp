
#pragma once

#include <QtCore>
#include <QtWidgets>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <QMultimedia>
#include <QMediaPlayer>
#include <QMediaPlaylist>

#include <QEvent>

#include "screensaver/screensaverinhibitor.hpp"
#include "compositionsuspender.hpp"
#include "qgraphicsbusybufferingitem.hpp"
#include "asssubtitlesitem.hpp"
#include "defs.hpp"

class VideoItem;

class PlayList;
class Player : public QGraphicsView
{
    Q_OBJECT
	Q_PROPERTY(QString VideoAspect MEMBER  VideoAspect)
	Q_PROPERTY(QSizeF VideoSize MEMBER  video_size)

public:

	Player(QWidget* parent = nullptr, bool use_opengl = true);
	~Player();

	// the the play play list, if none set, return empty list
	QMediaPlaylist * play_list(){
		return m_player.playlist();
	}

    QList<QGraphicsItem *> items(const QRect &rect, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const
    {
		auto r = QGraphicsView::items(rect, mode);
		r.removeAll(m_current_video_item);
		return r;
	}


Q_SIGNALS:
	void played(int index);

	void video_size_changed(QSizeF);

	// resize signal, then you might want to relocate/resize the widgets that you have add
	void resized(QSize);

	void media_stalled();
	void media_buffered();

	// 更新播放时间，可以在这里添加弹幕功能
	void time_stamp_updated(qreal);
	void time_stamp_fast_forward(qreal);

public Q_SLOTS:
	void set_play_list(QMediaPlaylist* list){m_player.setPlaylist(list);};

	void set_subtitle(QString subtitlefile);

	void play(){m_player.play();}
	void pause(){m_player.pause();}
	void stop(){m_player.stop();}

	void fast_forward(); // call this to forward 90s
	void fast_backwork(); // call this to back forward 90s

    void set_full_screen(bool v = true);

	void toogle_play_pause();

	void force_video_widget_size(QSizeF);


protected:
	virtual void resizeEvent(QResizeEvent*);

protected:
	std::pair<int, qint64> map_position_to_media(qint64);
	qint64 map_position_from_media(qint64);

private Q_SLOTS:
	void update_video_widget_size(QSizeF);
	void handle_resize(QSizeF);

	void slot_drag_slide(int);
	void slot_drag_slide_done();

	void slot_play_state_changed(QMediaPlayer::State);
	void slot_mediaStatusChanged(QMediaPlayer::MediaStatus);

	void slot_durationChanged(qint64);
	void slot_positionChanged(qint64);

	void slot_metaDataChanged(QString key,QVariant v);

private:
	QGraphicsScene m_scene;

	QScopedPointer<ScreenSaverInhibitor> m_screesave_inhibitor;
	QScopedPointer<CompositionSuspender> m_CompositionSuspender;

	// 进度条拖放位置
	int _drag_positoin = -1;

	// 播放控件们
	QMediaPlayer m_player;
	QGraphicsVideoItem* m_video_item_no_gl;
	VideoItem* m_video_item_gl;

	QGraphicsItem* m_current_video_item;

	QSlider	m_position_slide;
	QGraphicsProxyWidget* m_current_slide;

	QPointer<QGraphicsObject> play_indicator;
	QPointer<QGraphicsObject> pause_indicator;
	QGraphicsBusybufferingItem m_media_buffer_indicator;

	QString VideoAspect = "auto";
	QSizeF video_size;

protected:
	QPointer<AssSubtitlesItem> m_ass_item;
	VideoURLs urls;

};

