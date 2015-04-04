
#include "player.hpp"

#include <numeric>
#include <QGraphicsVideoItem>

#include <QOpenGLWidget>
#include "videoitem.hpp"
#include "graphicssvgitem.hpp"

#include "malloc.h"


Player::~Player()
{
	m_player.stop();
	m_current_slide->setWidget(0);

	m_scene.removeItem(&m_media_buffer_indicator);
}


Player::Player(QWidget* parent)
	: QGraphicsView(parent)
	, m_video_item_gl(0)
{
	setScene(&m_scene);
	setFocusPolicy(Qt::NoFocus);
	setCacheMode(QGraphicsView::CacheNone);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setContentsMargins(0,0,0,0);
	setBackgroundRole(QPalette::WindowText);

	m_player.setNotifyInterval(1000/23.976);

	m_scene.setBackgroundBrush(QBrush(QColor(0,0,0,255)));

	// 创建 opengl widget
	auto glwidget = new QOpenGLWidget(this);
	setViewport(glwidget);
	m_current_video_item = m_video_item_gl = new VideoItem;

	m_player.setVideoOutput(m_video_item_gl);

	QPalette Pal(palette());
	Pal.setColor(QPalette::Background, QColor(0,0,0,255));
	viewport()->setAutoFillBackground(true);
	viewport()->setPalette(Pal);
	viewport()->setBackgroundRole(QPalette::Background);

	// 	m_current_video_item->setZValue(-999);
	m_scene.addItem(m_current_video_item);

	scene()->addItem(&m_media_buffer_indicator);

	m_position_slide.setOrientation(Qt::Horizontal);
	// 绘制进度条君
	m_current_slide = m_scene.addWidget(&m_position_slide);
	// 先隐藏
// 	m_current_slide->hide();

	connect(&m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(slot_play_state_changed(QMediaPlayer::State)));
	connect(&m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slot_mediaStatusChanged(QMediaPlayer::MediaStatus)));

	connect(&m_player, SIGNAL(metaDataChanged(QString,QVariant)), this, SLOT(slot_metaDataChanged(QString,QVariant)));

	connect(&m_player, SIGNAL(positionChanged(qint64)), this, SLOT(slot_positionChanged(qint64)));
	connect(&m_player, SIGNAL(durationChanged(qint64)), this, SLOT(slot_durationChanged(qint64)));

	connect(&m_position_slide, SIGNAL(sliderMoved(int)), this, SLOT(slot_drag_slide(int)));
	connect(&m_position_slide, SIGNAL(sliderReleased()), this, SLOT(slot_drag_slide_done()));
}

void Player::set_subtitle(QString subtitlefile)
{
	delete m_ass_item;
	m_ass_item = new AssSubtitlesItem(subtitlefile);

	scene()->addItem(m_ass_item);
}

void Player::resizeEvent(QResizeEvent*e)
{
	QWidget::resizeEvent(e);
	handle_resize(e->size());
	resized(e->size());
}

void Player::update_video_widget_size(QSizeF rectsize)
{
	QSizeF video_widget_size = video_size;

	video_widget_size.scale(rectsize, Qt::KeepAspectRatio);
	QRectF video_rect(QPointF(), video_widget_size);


	video_rect.moveCenter(scene()->sceneRect().center());

	// 调整视频大小

	m_video_item_gl->setSize(video_widget_size);

	m_current_video_item->setPos(video_rect.topLeft());

	if (m_ass_item)
	{
		m_ass_item->update_video_size(video_widget_size);
		m_ass_item->setPos(video_rect.topLeft());
	}

}

void Player::handle_resize(QSizeF s)
{
	QRectF rect(QPointF(), s);
	m_scene.setSceneRect(rect);

	update_video_widget_size(s);

	QRectF slide_rect(QPointF(), QSizeF(rect.width(), m_position_slide.heightForWidth(rect.width())));

	slide_rect.setHeight(m_position_slide.sizeHint().height());

	slide_rect.moveBottomLeft(rect.bottomLeft());
	m_current_slide->setPos(slide_rect.topLeft());
	m_position_slide.resize(slide_rect.size().toSize());

	m_media_buffer_indicator.setPos(rect.center());

	if (m_current_slide->collidesWithItem(m_current_video_item))
	{
		m_current_slide->setOpacity(0.55);
	}
	else
	{
		m_current_slide->setOpacity(1.0);
	}

	if (pause_indicator)
		pause_indicator->setPos(rect.center());
	if (play_indicator)
		play_indicator->setPos(rect.center());
}

void Player::force_video_widget_size(QSizeF s)
{
	// force video widget to have the size s
// 	handle_resize(s);

	update_video_widget_size(s);
}

void Player::slot_durationChanged(qint64 duration)
{
	if (urls.size() > 1)
	{
		duration = std::accumulate(urls.begin(), urls.end(), 0, [](qint64 d, const VideoURL& u){
			return d + u.duration;
		});
	}
	// 更新进度条君.
	m_position_slide.setRange(0, duration);
}

void Player::slot_positionChanged(qint64 position)
{
	quint64 real_pos = map_position_from_media(position);
	if (_drag_positoin == -1)
	{
		m_position_slide.setValue(real_pos);
	}

	if (m_ass_item)
		m_ass_item->update_play_position(position);

	// 更改 tooltip

	auto current_play_time = QString("%1:%2:%3").arg(((real_pos/1000)/60)/60)
		.arg(QString::fromStdString(std::to_string(((real_pos/1000)/60) % 60)), 2, QChar('0'))
		.arg(QString::fromStdString(std::to_string((real_pos/1000) % 60)), 2, QChar('0'));

	bool tooltip_changed = m_position_slide.toolTip() != current_play_time;
	m_position_slide.setToolTip(current_play_time);

	if (tooltip_changed && m_position_slide.underMouse())
	{
		QToolTip::hideText();
		QToolTip::showText(QCursor::pos(), m_position_slide.toolTip());
	}

	// 播放弹幕.

	double time_stamp = real_pos / 1000.0;

	Q_EMIT time_stamp_updated(time_stamp);
}

void Player::slot_drag_slide(int p)
{
	_drag_positoin = p;

	auto current_play_time = QString("%1:%2:%3").arg(((_drag_positoin/1000)/60)/60)
		.arg(QString::fromStdString(std::to_string(((_drag_positoin/1000)/60) % 60)), 2, QChar('0'))
		.arg(QString::fromStdString(std::to_string((_drag_positoin/1000) % 60)), 2, QChar('0'));

	QToolTip::hideText();
	QToolTip::showText(QCursor::pos(), current_play_time);
}

void Player::slot_drag_slide_done()
{
	if (_drag_positoin != -1)
	{
		auto result = map_position_to_media(_drag_positoin);
		if (result.first == play_list()->currentIndex())
			m_player.setPosition(result.second);
		else
		{
			m_player.stop();
			play_list()->setCurrentIndex(result.first);
			m_player.load();
			m_player.setPosition(result.second);
			m_player.play();
		}
		time_stamp_fast_forward(_drag_positoin / 1000.0);
	}
	_drag_positoin = -1;
}

void Player::fast_backwork()
{
	_drag_positoin = m_position_slide.value() - 90000;
	slot_drag_slide_done();
}

void Player::fast_forward()
{
	_drag_positoin = m_position_slide.value() + 90000;
	slot_drag_slide_done();
}

std::pair< int, qint64 > Player::map_position_to_media(qint64 pos)
{
	int media_index = 0;

	if (urls.size() == 1 || urls.empty())
	{
		return std::make_pair(media_index, pos);
	}

	for (; media_index < urls.size(); media_index++)
	{
		const VideoURL & url = urls[media_index];
		if (pos > url.duration)
		{
			pos -= url.duration;
		}else
		{
			return std::make_pair(media_index, pos);
		}
	}

	return std::make_pair(media_index, pos);
}

qint64 Player::map_position_from_media(qint64 pos)
{
	if (urls.size() == 1)
		return pos;

	if(!play_list())
	{
		return pos;
	}

	if (play_list()->currentIndex() == 0)
		return pos;

	for (int i = 0; i < play_list()->currentIndex(); i++)
	{
		pos += urls[i].duration;
	}
	return pos;
}

void Player::slot_play_state_changed(QMediaPlayer::State state)
{
	switch(state)
	{
		case QMediaPlayer::PlayingState:
		{
			m_screesave_inhibitor.reset(new ScreenSaverInhibitor("bilibili-player", "playing videos"));
			raise();
			break;
		}
		case QMediaPlayer::PausedState:
		case QMediaPlayer::StoppedState:
		{
			m_screesave_inhibitor.reset();
		}
	}
}


void Player::slot_metaDataChanged(QString key, QVariant v)
{
	if (key == "Resolution")
	{
		// 计算比例。

		QSizeF new_video_size;

		if(VideoAspect=="auto")
		{
			new_video_size = v.toSize();
		}
		else
		{
			int w=0,h=0;
			std::sscanf(VideoAspect.toStdString().c_str(),  "%d:%d", &w,&h);

			QSizeF templatesize(w,h);

			templatesize.scale(v.toSizeF(), Qt::KeepAspectRatioByExpanding);

			new_video_size = templatesize;
		}

		if (new_video_size != video_size)
		{
			video_size = new_video_size;
			video_size_changed(video_size);
		}
	}
}

void Player::slot_mediaStatusChanged(QMediaPlayer::MediaStatus status)
{

	qDebug() << "media state :" << status;

	switch(status)
	{
		case QMediaPlayer::BufferedMedia:
			m_media_buffer_indicator.hide();
 			break;
		case QMediaPlayer::StalledMedia:
		case QMediaPlayer::BufferingMedia:
		{
			m_media_buffer_indicator.show();
			return;
		}

		break;
		case QMediaPlayer::NoMedia:
		case QMediaPlayer::EndOfMedia:
			m_media_buffer_indicator.hide();
			break;
		default:
			return;
	}
}


void Player::set_full_screen(bool is_full)
{

	if(is_full)
		window()->setWindowState( window()->windowState() | Qt::WindowFullScreen );
	else
		window()->setWindowState( window()->windowState() & ~Qt::WindowFullScreen );

	m_CompositionSuspender.reset();
	m_CompositionSuspender.reset(new CompositionSuspender(this));

	if (is_full)
	{
		m_current_slide->hide();

		setCursor(Qt::BlankCursor);

// 		if (allow_any_resize)
// 			adjust_window_size( m_mainwindow->windowHandle()->screen()->size());
	}
	else
	{
		m_current_slide->show();
		unsetCursor();
		setCursor(Qt::ArrowCursor);
	}
}

void Player::toogle_play_pause()
{
	switch (m_player.state())
	{
		case QMediaPlayer::PlayingState:
		{
			m_player.pause();

			// display 一个 pause 图标

			if (pause_indicator)
				pause_indicator->deleteLater();

			QGraphicsSvgItem * svg_item = new GraphicsSvgItem("://res/pause.svg");
			pause_indicator = svg_item;

			scene()->addItem(pause_indicator);

			auto effect = new QGraphicsOpacityEffect;

			pause_indicator->setGraphicsEffect(effect);

			auto ani_group = new QParallelAnimationGroup(svg_item);
			auto ani = new QPropertyAnimation(effect, "opacity", ani_group);
			auto ani_4 = new QPropertyAnimation(svg_item, "scale", ani_group);

			ani_group->addAnimation(ani);
			ani_group->addAnimation(ani_4);
			ani_4->setDuration(330);
			ani_4->setStartValue(3.0);
			ani_4->setEndValue(1.0);
			ani_4->setEasingCurve(QEasingCurve::OutBack);

			ani->setDuration(800);

			ani->setStartValue(0.0);
			ani->setEndValue(0.7);

			pause_indicator->setPos(scene()->sceneRect().center());
			pause_indicator->show();

			ani_group->start(QAbstractAnimation::DeleteWhenStopped);

			malloc_trim(0);

			break;
		}
		case QMediaPlayer::PausedState:
		{
			m_player.play();

			if (pause_indicator)
				pause_indicator->deleteLater();
			if (play_indicator)
				play_indicator->deleteLater();

			QGraphicsSvgItem * svg_item = new GraphicsSvgItem("://res/play.svg");
			play_indicator = svg_item;

			scene()->addItem(play_indicator);

			auto effect = new QGraphicsOpacityEffect;

			play_indicator->setGraphicsEffect(effect);

			auto ani_group = new QSequentialAnimationGroup(svg_item);

			auto ani_1 = new QPropertyAnimation(effect, "opacity", ani_group);

			ani_1->setDuration(50);

			ani_1->setStartValue(0.0);
			ani_1->setEndValue(0.7);

			auto ani_2 = new QPropertyAnimation(effect, "opacity", ani_group);

			ani_2->setDuration(300);

			ani_2->setStartValue(0.7);
			ani_2->setEndValue(0.6);

			auto ani_group_2 = new QParallelAnimationGroup(svg_item);

			auto ani_3 = new QPropertyAnimation(effect, "opacity", ani_group_2);
			auto ani_4 = new QPropertyAnimation(svg_item, "scale", ani_group_2);

			ani_3->setDuration(750);
			ani_3->setStartValue(0.6);
			ani_3->setEndValue(0.0);

			ani_4->setDuration(880);
			ani_4->setStartValue(1.0);
			ani_4->setEndValue(3.2);
			ani_4->setEasingCurve(QEasingCurve::InBack);

			ani_group->addAnimation(ani_1);
			ani_group->addAnimation(ani_2);

			ani_group_2->addAnimation(ani_3);
			ani_group_2->addAnimation(ani_4);

			ani_group->addAnimation(ani_group_2);

			connect(ani_group, SIGNAL(finished()), play_indicator, SLOT(deleteLater()));
			play_indicator->setPos(scene()->sceneRect().center());
			play_indicator->show();

			ani_group->start(QAbstractAnimation::DeleteWhenStopped);

			break;
		}
	}
}


