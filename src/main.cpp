
#include <iostream>
#include <QApplication>
#include <QString>
#include <QScreen>

#include "bplayer.hpp"
#include "bilibilires.hpp"

void fuckoff_low_dpi_screen(const QScreen* screen)
{
	std::cout << "screen DPI = " << screen->physicalDotsPerInch() << std::endl;
	std::cout << "screen Logical DPI = " << screen->logicalDotsPerInch() << std::endl;

	if ( screen->logicalDotsPerInch() < screen->physicalDotsPerInch())
	{
		std::cout << "you idiot! stupid dumb! Go fuck you self, have't you see the font tooo small for you?" << std::endl;
	}

	if( screen->devicePixelRatio() != 1.0)
	{
		std::cout << "do not set devicePixelRatio, you idiot" << std::endl;
		std::exit(1);
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
	cliparser.addOption({"opengl", "using opengl to render the video"});

	cliparser.process(app);

	fuckoff_low_dpi_screen(app.primaryScreen());

	if (cliparser.isSet("about-qt"))
	{
		app.aboutQt();
		return 0;
	}

	// argv[1] should by the url to play
 	QString bilibili_url = cliparser.positionalArguments().at(0);

	std::cerr << "play bilibili url: " << bilibili_url.toStdString() << std::endl;

	BPlayer player;

	if (cliparser.isSet("use-bullet"))
	{
		player.setProperty("UseBullet", cliparser.value("use-bullet") != "no");
	}

	if (cliparser.isSet("opengl"))
	{
		player.setProperty("UseOpenGL", cliparser.value("opengl") != "no");
	}

	auto bilibili_res = new BiliBiliRes(bilibili_url.toStdString());

	if (cliparser.isSet("videourl"))
	{
		bilibili_res->setProperty("DoNotExtractVideoUrl", true);

		VideoURL url;
		url.url = cliparser.value("videourl").toStdString();
		player.append_video_url(url);
	}else
	{
		QObject::connect(bilibili_res, SIGNAL(video_url_extracted(VideoURL)), &player, SLOT(append_video_url(VideoURL)));
	}

	QObject::connect(bilibili_res, SIGNAL(barrage_extracted(QDomDocument)), &player, SLOT(set_barrage_dom(QDomDocument)));
	QObject::connect(bilibili_res, SIGNAL(finished()), &player, SLOT(start_play()));
	QObject::connect(bilibili_res, SIGNAL(finished()), bilibili_res, SLOT(deleteLater()));

	app.exec();

	return 0;
}

