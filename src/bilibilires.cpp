
#include <iostream>
#include <boost/regex.hpp>

#include "bilibilires.hpp"

static const std::string APPKEY = "85eb6835b0a1034e";
static const std::string SECRETKEY = "2ad42749773c441109bdc0191257a664";

static std::string calc_sign(std::string str)
{
	unsigned char result[MD5_DIGEST_LENGTH];

	MD5(reinterpret_cast<const unsigned char*>(str.c_str()), str.length(), result);

	std::string ret;
	ret.reserve(32);

	for( int i = 0; i < MD5_DIGEST_LENGTH; i++ )
	{
		char hex[3];
		std::snprintf(hex, 3, "%02x", result[i]);
		ret.append(hex ,2);
	}
	return ret;
}

// sign_this = calc_sign('appkey={APPKEY}&cid={cid}&quality=4{SECRETKEY}'.format(APPKEY = APPKEY, cid = cid, SECRETKEY = SECRETKEY))
static std::string get_sign_for_play_url(std::string cid)
{
	auto str = boost::str(boost::format("appkey=%s&cid=%s&quality=4%s") % APPKEY % cid % SECRETKEY);
	return calc_sign(str);
}

static std::string get_video_url(const std::string& cid)
{
	auto formated_url = boost::format("http://interface.bilibili.com/playurl?appkey=%s&cid=%s&quality=4%s&sign=%s")
		% APPKEY % cid % SECRETKEY % get_sign_for_play_url(cid);

 	return boost::str(formated_url);
}

static std::string get_comment_url(const std::string& aid)
{
	return boost::str(boost::format("http://comment.bilibili.com/%s.xml") % aid);
}

void BiliBiliRes::extract_aid_cid_stuff()
{
    std::string real_player_page = current_reply->readAll().toStdString();

    // extract aid and cid from url
    boost::smatch what;

    if (boost::regex_search(real_player_page, what, boost::regex("cid=([0-9]+)&aid=([0-9]+)")))
    {
        cid = what[1];
        aid = what[2];
        aid_extracted(QString::fromStdString(aid));
        cid_extracted(QString::fromStdString(cid));
    } else if (boost::regex_search(real_player_page, what, boost::regex("bili-cid=([0-9]+)&bili-aid=([0-9]+)")))
    {
        cid = what[1];
        aid = what[2];
        aid_extracted(QString::fromStdString(aid));
        cid_extracted(QString::fromStdString(cid));
    }
    else if (boost::regex_search(real_player_page, what, boost::regex("bili-cid=([0-9]+)&")))
    {
        cid = what[1];
        cid_extracted(QString::fromStdString(cid));
    }
}

void BiliBiliRes::slot_cid_extracted(QString cid)
{
    auto play_url = get_video_url(cid.toStdString());

    std::cout << "getting playurl from : " << play_url << std::endl;

    // get aid and cid from url
    QNetworkReply * reply = current_reply = m_netmgr.get(QNetworkRequest(QUrl(play_url.c_str())));

    connect(reply, SIGNAL(finished()), this, SLOT(extract_flv_url()));
}

void BiliBiliRes::extract_flv_url()
{
    QDomDocument m_xml;

    m_xml.setContent(current_reply->readAll());

    // now get url
    QDomNodeList durls = m_xml.elementsByTagName("durl");

    for (int i = 0; i < durls.size(); i++)
    {

        QDomElement url = durls.at(i).firstChildElement("url");
        QString data = url.text();

        BiliBili_VideoURL vurl;

        vurl.order = i;
        vurl.url = url.text().toStdString();

        vurl.duration = url.firstChildElement("length").text().toULongLong();

        auto backup_urls = url.firstChildElement("backup_url").elementsByTagName("url");

        for (int i=0; i < backup_urls.size(); i++)
        {
            QDomElement backup_url = backup_urls.at(i).toElement();
            vurl.backup_urls.push_back(backup_url.text().toStdString());
        }

        video_url_extracted(vurl);
    }

    auto xml_url = get_comment_url(cid);
    std::cout << "getting barrage from : " << xml_url << std::endl;

    barrage_url_extracted(QString::fromStdString(xml_url));
}
