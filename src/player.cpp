
#include "player.hpp"

#include <QGraphicsVideoItem>
#include <QOpenGLWidget>
#include "videoitem.hpp"


Player::Player(QWidget* parent, bool use_opengl)
	: QWidget(parent)
	, m_scene(this)
	, m_view(this)
{
	m_view.setScene(&m_scene);
	m_view.setFocusPolicy(Qt::NoFocus);
	m_view.setCacheMode(QGraphicsView::CacheNone);
	m_view.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	m_view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view.setContentsMargins(0,0,0,0);
	m_view.setBackgroundRole(QPalette::WindowText);

	if (use_opengl)
	{
		m_current_video_item = m_video_item_gl = new VideoItem;

		// 创建 opengl widget
		auto glwidget = new QOpenGLWidget(&m_view);

		QSurfaceFormat format;

		format.setProfile(QSurfaceFormat::CompatibilityProfile);
		format.setRenderableType(QSurfaceFormat::OpenGL);

		format.setVersion(3,0);

		format.setSamples(0);

		format.setSwapBehavior(QSurfaceFormat::SingleBuffer);
		QSurfaceFormat::setDefaultFormat(format);

		glwidget->setFormat(format);

		m_view.setViewport(glwidget);

		m_player.setVideoOutput(m_video_item_gl);
	}
	else
	{
		m_current_video_item = m_video_item_no_gl = new QGraphicsVideoItem;
		m_player.setVideoOutput(m_video_item_no_gl);
	}

	m_current_video_item->setZValue(-999);
	m_scene.addItem(m_current_video_item);

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
	QRect rect(QPoint(), e->size());

	m_scene.setSceneRect(rect);
	m_view.setGeometry(rect);
	resized(e->size());
}

void Player::slot_durationChanged(qint64 dur)
{
	// 更新进度条君.

// 	m_position_slide.setTickPosition();

}

