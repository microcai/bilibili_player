
#include <iostream>
#include <QApplication>
#include <QString>
#include <QScreen>
#include <QMessageBox>
#include <QSystemTrayIcon>

#ifdef QT_X11EXTRAS_LIB
#include <QtX11Extras>
#include <QX11Info>
#include "xrandr/xrandr.hpp"
#endif

#include "bplayer.hpp"
#include "bilibilires.hpp"
#include "player.hpp"

static void fuckoff_low_dpi_screen(const QScreen* screen, QSize native_screen_size)
{
	bool can_notify = QSystemTrayIcon::supportsMessages();

	std::cout << "detecting screen ... " << std::endl;

	if (native_screen_size.isValid())
	{
		std::cout << "screen resulution: " << native_screen_size.width() << "x" << native_screen_size.height() << std::endl;
	}

	if (native_screen_size.isValid() && screen->size()!=native_screen_size)
	{
		QString msg = QString("屏幕没有设定到最佳分辨率，最佳分辨率是 %1x%2，请使用最佳分辨率以提高画质").arg(native_screen_size.width()).arg(native_screen_size.height());
		if (can_notify)
		{
			QSystemTrayIcon tray;
			tray.show();
			tray.showMessage("注意注意！", msg, QSystemTrayIcon::Warning);

			qApp->processEvents();
			QThread::sleep(3);
		}
		else
		{
			QMessageBox box;
			box.setText(msg);
			box.exec();
		}
	}

	std::cout << "screen DPI = " << screen->physicalDotsPerInch() << " " << [](qreal dpi){
		if ( dpi < 90)
		 return "bad screen :(";
		if ( dpi > 100 && dpi < 150)
			return "good!";
		return "you got a nice screen!";
	}(screen->physicalDotsPerInch()) << std::endl;

	std::cout << "screen Logical DPI = " << screen->logicalDotsPerInch() << " " << [](qreal dpi){
		if ( dpi >= 192)
		{
			return "you're smart guy!";
		}
		return "stupid DPI settings!";
	}(screen->logicalDotsPerInch()) << std::endl;

	[can_notify](qreal logicaldpi, qreal physicaldpi)
	{
		if ( logicaldpi < physicaldpi)
		{
			std::cout << "you idiot! stupid dumb! Go fuck you self, have't you see the font tooo small for you?" << std::endl;

			QString msg = QString("你的屏幕实际 DPI 为 %1, 但是系统才设置 %2 的 DPI, 难道你不觉得字体很小看着不舒服么？\n"
			"赶紧打开系统设置，将 DPI 设置的比屏幕 DPI 大点！建议设置为 %3").arg(physicaldpi).arg(logicaldpi)
			.arg([](qreal phydpi){
				if ( phydpi < 96)
					return 96;
				if ( phydpi < 115)
					return 120;
				if  (phydpi < 130)
					return 144;
				if (phydpi <= 182)
					return 192;
				if (phydpi <= 230)
					return 240;
				if(phydpi <= 260)
					return 288;
				if (phydpi < 330)
					return 384;
				if (phydpi < 500)
					return 576;

				return (int(qRound(phydpi+1))/ 8 + 1) * 8;
			}(physicaldpi));

			if (can_notify)
			{
				QSystemTrayIcon tray;
				tray.show();
				tray.showMessage("注意注意！", msg, QSystemTrayIcon::Critical);
			}
			else
			{
				QMessageBox box;
				box.setText(msg);
				box.exec();
			}
		}
	}(screen->logicalDotsPerInch(), screen->physicalDotsPerInch());

	if( screen->devicePixelRatio() != 1.0)
	{
		std::cout << "do not set devicePixelRatio, you idiot" << std::endl;
	}
}

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QCoreApplication::setApplicationName("bilibili player");
	QCoreApplication::setApplicationVersion("0.9");

	QIcon exe_icon(":/ui/bilibili.ico");

	app.setWindowIcon(exe_icon);

	QCommandLineParser cliparser;
	cliparser.setApplicationDescription("bilibili 播放器");

	cliparser.addHelpOption();
	cliparser.addVersionOption();
	cliparser.addOption({"about-qt", "display about-qt dialog"});
	cliparser.addOption({"about", "display about dialog"});

	cliparser.addOption({"use-bullet", "use bullet engine to manage danmaku"});
	cliparser.addOption({"videourl", "alternative video url, useful for play local video file while still  be able to see danmaku", "uri"});
	cliparser.addOption({"ass", "load subtitle from this ass", "path"});
	cliparser.addOption({"nogl", "do not using opengl to render the video and danmaku"});
	cliparser.addOption({"no-minimalsize", "allow resize freely"});
	cliparser.addOption({"force-aspect", "force video aspect", "16:9"});
	cliparser.addOption({"ass", "load ass file", "file"});

	cliparser.process(app);

	QSize native_screen_size;
#ifdef QT_X11EXTRAS_LIB
	if (QX11Info::isPlatformX11())
	{
		// 加入 xrandr 检查是否使用了最佳分辨率.
		native_screen_size = native_res_for_monitior();
	}
#endif


	fuckoff_low_dpi_screen(app.primaryScreen(), native_screen_size);

	if (cliparser.isSet("about-qt"))
	{
		app.aboutQt();
		return 0;
	}

	QString bilibili_url;
	// argv[1] should by the url to play

	if (cliparser.positionalArguments().size() >= 1)
	{
		bilibili_url = cliparser.positionalArguments().at(0);
		std::cerr << "play bilibili url: " << bilibili_url.toStdString() << std::endl;

	}else
	{
		std::cerr << "\n\n\n -- 必须要有 bilibili 地址哦！ -- \n 以下是帮助" << std::endl;

		cliparser.showHelp(1);
	}

	QMediaPlaylist playlist;
	playlist.setPlaybackMode(QMediaPlaylist::Sequential);

	BPlayer player((cliparser.value("nogl") != "no"));
	player.set_play_list(&playlist);

	if (cliparser.isSet("use-bullet"))
	{
		player.setProperty("UseBullet", cliparser.value("use-bullet") != "no");
	}

	if (cliparser.isSet("force-aspect"))
	{
		player.setProperty("VideoAspect", cliparser.value("force-aspect"));

		qDebug() << "force aspect to :" <<  cliparser.value("force-aspect");
	}

	if (cliparser.isSet("no-minimalsize"))
	{
		player.setProperty("AllowAnySize", (cliparser.value("no-minimalsize") != "no"));
	}

	if (cliparser.isSet("ass"))
	{
		player.set_subtitle(cliparser.value("ass"));
	}

	auto bilibili_res = new BiliBiliRes(bilibili_url.toStdString());

	if (cliparser.isSet("videourl"))
	{
		bilibili_res->setProperty("DoNotExtractVideoUrl", true);

		VideoURL url;
		url.url = cliparser.value("videourl").toStdString();
		player.append_video_url(url);

		playlist.addMedia(QUrl(cliparser.value("videourl")));

	}else
	{
		QObject::connect(bilibili_res, SIGNAL(video_url_extracted(VideoURL)), &player, SLOT(append_video_url(VideoURL)));
	}

	QObject::connect(bilibili_res, SIGNAL(finished()), bilibili_res, SLOT(deleteLater()));

	QObject::connect(bilibili_res, SIGNAL(barrage_extracted(QDomDocument)), &player, SLOT(set_barrage_dom(QDomDocument)));
	QObject::connect(bilibili_res, SIGNAL(finished()), &player, SLOT(start_play()));
	QObject::connect(bilibili_res, SIGNAL(finished()), &player, SLOT(show()));

	app.exec();

	return 0;
}

