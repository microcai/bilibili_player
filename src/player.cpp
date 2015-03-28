
#include "player.hpp"

#include <QGraphicsVideoItem>
#include <QOpenGLWidget>
#include "videoitem.hpp"

Player::~Player()
{
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

	// 绘制进度条君
	m_current_slide = m_scene.addWidget(&m_position_slide);
	// 先隐藏
	m_current_slide->hide();

// connect m_player signals

	connect(&m_player, SIGNAL(durationChanged(qint64)), SLOT(slot_durationChanged(qint64)));
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

	resized(e->size());
}

void Player::slot_durationChanged(qint64 dur)
{
	// 更新进度条君.

// 	m_position_slide.setTickPosition();

}

