
#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <ctime>

#include <QColor>

struct BiliBili_VideoURL
{
	std::string url;
	std::vector<std::string> backup_urls;

	int order;
	uint64_t duration;
};

typedef std::vector<BiliBili_VideoURL> BiliBili_VideoURLs;

struct BiliBili_Comment
{
	std::string content;

	double time_stamp; // in video

	enum { Normal = 1, Button = 4, Top = 5, Reverse = 6, Positioned = 7, Advanced = 8 } mode;

	double font_size;

	QColor font_color; // as HTML code

	std::time_t post_time; // 发表时间, in UNIX EPOH

	enum { Average = 0, Subtitle = 1, Special = 2} type;

	std::string poster;

	uint64_t rowID;
};

typedef std::vector<BiliBili_Comment> BiliBili_Comments;
