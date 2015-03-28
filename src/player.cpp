
#include "player.hpp"

#include <QGraphicsVideoItem>
#include <QOpenGLWidget>
#include "videoitem.hpp"

Player::~Player()
{
	m_player.stop();
	m_current_slide->setWidget(0);

	m_scene.removeItem(&m_media_buffer_indicator);
}


Player::Player(QWidget* parent, bool use_opengl)
	: QGraphicsView(parent)
	, m_video_item_gl(0)
	, m_video_item_no_gl(0)
{
	setScene(&m_scene);
	setFocusPolicy(Qt::NoFocus);
	setCacheMode(QGraphicsView::CacheNone);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setContentsMargins(0,0,0,0);
	setBackgroundRole(QPalette::WindowText);

	if (use_opengl)
	{
		m_current_video_item = m_video_item_gl = new VideoItem;
		m_scene.addItem(m_video_item_gl);

		// 创建 opengl widget
		auto glwidget = new QOpenGLWidget(this);
		setViewport(glwidget);

		m_player.setVideoOutput(m_video_item_gl);
	}
	else
	{
		m_current_video_item = m_video_item_no_gl = new QGraphicsVideoItem;
		m_scene.addItem(m_video_item_no_gl);
		m_player.setVideoOutput(m_video_item_no_gl);
	}

// 	m_current_video_item->setZValue(-999);

	scene()->addItem(&m_media_buffer_indicator);

	m_position_slide.setOrientation(Qt::Horizontal);
	// 绘制进度条君
	m_current_slide = m_scene.addWidget(&m_position_slide);
	// 先隐藏
// 	m_current_slide->hide();

	connect(&m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(slot_play_state_changed(QMediaPlayer::State)));
	connect(&m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slot_mediaStatusChanged(QMediaPlayer::MediaStatus)));

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
	QRectF rect(QPointF(), e->size());
	m_scene.setSceneRect(rect);

	QSizeF video_widget_size(16,9);
// 	QSizeF video_widget_size = e->size();

	video_widget_size.scale(rect.size(), Qt::KeepAspectRatio);

	QRectF video_rect(QPointF(), video_widget_size);

	video_rect.moveCenter(rect.center());
	if (m_ass_item)
		m_ass_item->update_video_size(video_widget_size);

	// 调整视频大小
	if(m_video_item_gl)
	{
		m_video_item_gl->setSize(video_widget_size);
	}
	if(m_video_item_no_gl)
	{
		m_video_item_no_gl->setSize(video_widget_size);
	}

	m_current_video_item->setPos(video_rect.topLeft());

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

	resized(e->size());
}

void Player::slot_durationChanged(qint64 duration)
{
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

// 		QPoint tooltip_pos = m_mainwindow->mapFromGlobal(QCursor::pos());
		QToolTip::showText(QCursor::pos(), m_position_slide.toolTip());
	}


	// 播放弹幕.

	double time_stamp = real_pos / 1000.0;

	Q_EMIT time_stamp_updated(time_stamp);

// 	while (m_comment_pos != m_comments.end())
// 	{
// 		const Moving_Comment & c = * m_comment_pos;
// 		if (c.time_stamp < time_stamp)
// 		{
// 			m_comment_pos ++;
//
// 			// 添加弹幕.
//
// 			if (c.type ==0)
// 			{
// 				add_barrage(c);
// 			}
//
// 		}else
// 			break;
// 	}
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

void Player::slot_mediaStatusChanged(QMediaPlayer::MediaStatus status)
{

	qDebug() << "media state :" << status;

	switch(status)
	{
		case QMediaPlayer::BufferedMedia:
			m_media_buffer_indicator.hide();
 			break;
		case QMediaPlayer::StalledMedia:
		{
			m_media_buffer_indicator.show();
			return;
		}

		break;
		case QMediaPlayer::NoMedia:
			m_media_buffer_indicator.hide();
			break;
		default:
			m_media_buffer_indicator.show();
	}
}


void Player::set_full_screen(bool is_full)
{
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
