#include <iostream>

#include <malloc.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <ctime>

#include <QObject>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QString>
#include <QWindow>
#include <QMainWindow>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsTextItem>
#include <QApplication>
#include <QPropertyAnimation>
#include <QGraphicsSvgItem>
#include <QScreen>
#include <QToolTip>
#include <QDesktopWidget>
#include <QOpenGLWidget>
#include <QGraphicsVideoItem>
#include <QSurfaceFormat>
#include <QResizeEvent>
#include <QOpenGLFunctions>
#include <QTime>
#include <boost/regex.hpp>

#include "bplayer.hpp"
#include "bilibilires.hpp"


class MyMainWindow : public QMainWindow
{
	Q_OBJECT
public:
    explicit MyMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0)
		:QMainWindow(parent, flags)
	{
	}

	virtual void paintEvent(QPaintEvent*)
	{
		QPainter p(this);

		QOpenGLFunctions gl;

		gl.initializeOpenGLFunctions();

// 		QOpenGLFun

		p.beginNativePainting();

		gl.glClearColor(0,0,0,1);

		gl.glClear(GL_COLOR_BUFFER_BIT);

		p.end();
	}

	virtual void resizeEvent(QResizeEvent* e)
	{
		resized(e->size(), e->oldSize());
		QMainWindow::resizeEvent(e);
	}
Q_SIGNALS:
	void resized(QSizeF newsize, QSizeF oldsize);
};


static Moving_Comments to_comments(const QDomDocument& barrage)
{
	Moving_Comments m_comments;
	// now we got 弹幕, start dumping it!

	// 先转换成好用点的格式.

	auto ds = barrage.elementsByTagName("d");

	m_comments.reserve(ds.size());

	for (int i=0; i< ds.size(); i++)
	{
		Moving_Comment c;
		auto p = ds.at(i).toElement();
		c.content = p.text().toStdString();

		auto format_string = p.attribute("p").toStdString();

		std::vector<std::string> format_string_splited;

		boost::regex_split(std::back_inserter(format_string_splited), format_string, boost::regex(","));

		c.time_stamp = boost::lexical_cast<double>(format_string_splited[0]);

		///format_string_splited[0]

		c.mode = static_cast<decltype(c.mode)>(boost::lexical_cast<int>(format_string_splited[1]));

		c.font_size = boost::lexical_cast<double>(format_string_splited[2]);

		c.font_color.setRgb(boost::lexical_cast<uint32_t>(format_string_splited[3]));

		c.post_time = boost::lexical_cast<uint64_t>(format_string_splited[4]);

		c.type = static_cast<decltype(c.type)>(boost::lexical_cast<int>(format_string_splited[5]));

		c.poster = format_string_splited[6];
		c.rowID = boost::lexical_cast<uint64_t>(format_string_splited[7]);

		m_comments.push_back(c);
	}

	std::sort(m_comments.begin(), m_comments.end(), [](const Moving_Comment& a, const Moving_Comment& b) -> bool{
		return a.time_stamp < b.time_stamp;
	});

	m_comments.capacity();

	return m_comments;
}

BPlayer::BPlayer(QWidget* parent)
	: Player(parent)
{
}

BPlayer::~BPlayer()
{
}

void BPlayer::append_video_url(VideoURL url)
{
	// now we got video uri, start playing!
	QString current_url = QString::fromStdString(url.url);

	play_list()->addMedia(QUrl(current_url));

	// set the title
	urls.push_back(url);
}

void BPlayer::set_barrage_dom(QDomDocument barrage)
{
	m_comments = to_comments(barrage);
	m_comment_pos = m_comments.begin();
}

void BPlayer::start_play()
{
	QTimer::singleShot(2000, this, SLOT(play()));

	connect(play_list(), SIGNAL(currentIndexChanged(int)), this, SLOT(slot_mediaChanged(int)));

	QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::FullScreen), this);
	connect(shortcut, SIGNAL(activated()), this, SLOT(toggle_full_screen_mode()));
	shortcut = new QShortcut(QKeySequence("f"), this);
	connect(shortcut, SIGNAL(activated()), this, SLOT(toggle_full_screen_mode()));

	shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
	connect(shortcut, SIGNAL(activated()), this, SLOT(toogle_play_pause()));

	shortcut = new QShortcut(QKeySequence(QKeySequence::ZoomOut), this);
	connect(shortcut, SIGNAL(activated()), this, SLOT(zoom_out()));
	shortcut = new QShortcut(QKeySequence(QKeySequence::ZoomIn), this);
	connect(shortcut, SIGNAL(activated()), this, SLOT(zoom_in()));

	shortcut = new QShortcut(QKeySequence("Ctrl+1"), this);
	connect(shortcut, &QShortcut::activated, [this](){
		SetZoomLevel(1.0);
	});
	shortcut = new QShortcut(QKeySequence("Ctrl+2"), this);
	connect(shortcut, &QShortcut::activated, [this](){
		SetZoomLevel(2.0);
	});
	shortcut = new QShortcut(QKeySequence("Ctrl+3"), this);
	connect(shortcut, &QShortcut::activated, [this](){
		SetZoomLevel(3.0);
	});
	shortcut = new QShortcut(QKeySequence("Ctrl+4"), this);
	connect(shortcut, &QShortcut::activated, [this](){
		SetZoomLevel(4.0);
	});
	shortcut = new QShortcut(QKeySequence("Ctrl+5"), this);
	connect(shortcut, &QShortcut::activated, [this](){
		SetZoomLevel(5.0);
	});
	shortcut = new QShortcut(QKeySequence("Ctrl+6"), this);
	connect(shortcut, &QShortcut::activated, [this](){
		SetZoomLevel(6.0);
	});

	shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right), this);
	connect(shortcut, SIGNAL(activated()), this, SLOT(fast_forward()));

	shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this);
	connect(shortcut, SIGNAL(activated()), this, SLOT(fast_backwork()));

	connect(this, SIGNAL(time_stamp_updated(qreal)), this, SLOT(play_position_update(qreal)));
	connect(this, SIGNAL(time_stamp_fast_forward(qreal)), this, SLOT(play_position_fast_forwarded(qreal)));

	connect(this, SIGNAL(video_size_changed(QSizeF)), this, SLOT(slot_video_size_changed(QSizeF)));
}

void BPlayer::add_barrage(const Moving_Comment& c)
{
	QFont font;
	font.setPointSizeF(c.font_size /1.5);
	font.setFamily("Sans");

	auto vsize = scene()->sceneRect().size();

	auto effect =  new QGraphicsDropShadowEffect();
	effect->setOffset(3);
	effect->setBlurRadius(5);
	effect->setEnabled(1);
	effect->setColor(QColor::fromRgb(0,0,0));

	QGraphicsTextItem * danmu = scene()->addText(QString::fromStdString(c.content));

	danmu->setFont(font);
	danmu->setDefaultTextColor(c.font_color);

	danmu->setGraphicsEffect(effect);

	auto preferedY = lastY += danmu->boundingRect().height() + logicalDpiY()  / 72.0 * 3 ;

	auto textWidth = danmu->boundingRect().width();

	if ( lastY > vsize.height()*0.66)
		lastY = 0;

	if (use_bullet)
	{
		QTransform qtrans;
		qtrans.translate(vsize.width(), preferedY);
		danmu->setTransform(qtrans);
		m_danmumgr.add_danmu(danmu);
		return;
	}

	QTime spend_time;

	spend_time.start();

	if ( lastY > vsize.height() * 0.22)
	{
		// 应该开始寻找替代位置
		for (int guessY = 6; (guessY < vsize.height() * 0.7) && (spend_time.elapsed() < 500) ; guessY++)
		{
			QRect rect(vsize.width() - textWidth * 0.7, guessY, textWidth, danmu->boundingRect().height() + logicalDpiY() / 72.0 * 2);
			auto items = QGraphicsView::items(rect, Qt::IntersectsItemShape);

			if (items.empty())
			{
				preferedY = guessY;
				break;
			}else if (items.size() == 1)
			{
				if (items.at(0) == danmu)
				{
					preferedY = guessY;
					break;
				}
			}

			auto bottom_item_it  = std::max_element(items.begin(), items.end(), [](QGraphicsItem* itema,QGraphicsItem* itemb){
				return itema->boundingRect().bottom() < itemb->boundingRect().bottom();
			});

			if (bottom_item_it != items.end())
			{
				lastY = (*bottom_item_it)->boundingRect().bottom();
			}
		}
	}


	QTransform qtrans;
	qtrans.translate(vsize.width(), preferedY);
	danmu->setTransform(qtrans);

	QVariantAnimation *animation = new QVariantAnimation(danmu);
	connect(animation, SIGNAL(finished()), danmu, SLOT(deleteLater()));

	animation->setStartValue(vsize.width());
	animation->setEndValue((qreal)0.0 - textWidth);
	animation->setDuration(vsize.width() * 6);

	connect(animation, &QVariantAnimation::valueChanged, danmu, [danmu](const QVariant& v)
	{
		auto dy = danmu->transform().dy();

		QTransform qtrans;

		qtrans.translate(v.toReal(), dy);

		danmu->setTransform(qtrans);
	});
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}


void BPlayer::toggle_full_screen_mode()
{
	if (window()->isFullScreen())
	{
		set_full_screen(false);
	}else{
		set_full_screen(true);
	}
}

void BPlayer::slot_video_size_changed(QSizeF video_size)
{
	if (qIsNaN(zoom_level))
	{
		// 根据屏幕大小决定默认的缩放比例.
		QSize desktopsize = qApp->desktop()->availableGeometry().size();

		zoom_level = qMin(
			desktopsize.height() / video_size.height()
			,
			desktopsize.width() / video_size.width()
		);

		if (zoom_level <= 1.0)
			zoom_level = 1.0;
		else
			zoom_level = (long)(zoom_level);

		SetZoomLevel(zoom_level);
		updateGeometry();
	}
}

void BPlayer::slot_mediaChanged(int)
{
	std::cout << "playing: " << play_list()->currentMedia().canonicalUrl().toDisplayString().toStdString() << std::endl;
}

void BPlayer::play_position_fast_forwarded(qreal time_stamp)
{
	m_comment_pos = m_comments.begin();

	while (m_comment_pos != m_comments.end())
	{
		const Moving_Comment & c = * m_comment_pos;
		if (c.time_stamp > time_stamp)
			break;
		m_comment_pos ++;
	}
}

void BPlayer::play_position_update(qreal time_stamp)
{
	QTime avoid_long_lock;
	avoid_long_lock.start();
	// 播放弹幕.
	while ((avoid_long_lock.elapsed() < 2000) && m_comment_pos != m_comments.end())
	{
		const Moving_Comment & c = * m_comment_pos;
		if (c.time_stamp < time_stamp)
		{
			m_comment_pos ++;

			// 添加弹幕.

			if (c.type ==0)
			{
				add_barrage(c);
			}

		}else
			break;
	}

	while ( m_comment_pos != m_comments.end())
	{
		const Moving_Comment & c = * m_comment_pos;
		if (c.time_stamp < time_stamp)
		{
			m_comment_pos ++;
		}else
			break;
	}
}


#include "bplayer.moc"
