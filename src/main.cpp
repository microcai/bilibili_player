
#include <iostream>
#include <QApplication>
#include <QScreen>

#include "bplayer.hpp"
#include "bilibilires.hpp"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);


	const QScreen& screen = *app.primaryScreen();

	std::cout << "screen DPI = " << screen.physicalDotsPerInch() << std::endl;
	std::cout << "screen Logical DPI = " << screen.logicalDotsPerInch() << std::endl;

	if ( screen.logicalDotsPerInch() < screen.physicalDotsPerInch())
	{
		std::cout << "you idiot! stupid dumb! Go fuck you self, have't you see the font tooo small for you?" << std::endl;
	}

	if( screen.devicePixelRatio() != 1.0)
	{
		std::exit(1);
	}

	// argv[1] should by the url to play
 	std::string bilibili_url = argv[1];

	std::cerr << "play bilibili url: " << bilibili_url << std::endl;

	auto bilibili_res = new BiliBiliRes(bilibili_url);

	BPlayer player;

	QObject::connect(bilibili_res, SIGNAL(video_url_extracted(VideoURL)), &player, SLOT(append_video_url(VideoURL)));
	QObject::connect(bilibili_res, SIGNAL(barrage_extracted(QDomDocument)), &player, SLOT(set_barrage_dom(QDomDocument)));
	QObject::connect(bilibili_res, SIGNAL(finished()), &player, SLOT(start_play()));
	QObject::connect(bilibili_res, SIGNAL(finished()), bilibili_res, SLOT(deleteLater()));

	app.exec();

	return 0;
}

