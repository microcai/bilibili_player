
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

class VideoItem;

class PlayList;
class Player : public QWidget
{
    Q_OBJECT
public:

	Player(QWidget* parent, bool use_opengl = false);

	// the scene used by player
	QGraphicsScene* scene() const {
		return const_cast<QGraphicsScene*>(&m_scene);
	}
	QGraphicsView* view() const {
		return const_cast<QGraphicsView*>(&m_view);
	}

	// the the play play list, if none set, return empty list
	QMediaPlaylist * play_list(){
		return m_player.playlist();
	}

	void set_play_list(QMediaPlaylist* list){m_player.setPlaylist(list);};

Q_SIGNALS:
	void played(int index);

	// resize signal, then you might want to relocate/resize the widgets that you have add
	void resized(QSize);

	void media_stalled();
	void media_buffered();


public Q_SLOTS:
	void play(){m_player.play();}
	void pause(){m_player.pause();}
	void stop(){m_player.stop();}

protected:
	virtual void resizeEvent(QResizeEvent*);

private Q_SLOTS:

	void slot_durationChanged(qint64);

private:
	QScopedPointer<ScreenSaverInhibitor> m_screesave_inhibitor;
	QScopedPointer<CompositionSuspender> m_CompositionSuspender;

	QMediaPlayer m_player;

	QGraphicsVideoItem* m_video_item_no_gl;
	VideoItem* m_video_item_gl;

	QGraphicsItem* m_current_video_item;

	QSlider	m_position_slide;
	QGraphicsProxyWidget* m_current_slide;

	QGraphicsView m_view;
	QGraphicsScene m_scene;
};

