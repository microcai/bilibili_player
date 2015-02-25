
#include <iostream>
#include <QApplication>

#include "bplayer.hpp"
#include "bilibilires.hpp"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

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

