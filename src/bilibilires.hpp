
#pragma once

#include <string>
#include <boost/format.hpp>
#include <openssl/md5.h>

#include <QObject>

#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>

#include "bilibilidef.hpp"


class BiliBiliRes : public QObject
{
	Q_OBJECT
public:
	explicit BiliBiliRes(const std::string& bilibili_url)
		: m_bilibili_url(bilibili_url)
	{
		connect(&m_netmgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkreplyed(QNetworkReply*)), Qt::QueuedConnection);

		// get aid and cid from url
		QNetworkReply * reply = current_reply = m_netmgr.get(QNetworkRequest(QUrl(bilibili_url.c_str())));

		connect(reply, SIGNAL(finished()), this, SLOT(extract_aid_cid_stuff()));

		connect(this, SIGNAL(cid_extracted(QString)), this, SLOT(slot_cid_extracted(QString)));
		connect(this, SIGNAL(aid_extracted(QString)), this, SLOT(slot_aid_extracted(QString)));
		connect(this, SIGNAL(barrage_url_extracted(QString)), this, SLOT(slot_barrage_url_extracted(QString)));
	}

	virtual ~BiliBiliRes(){};

Q_SIGNALS:
	void video_url_extracted(BiliBili_VideoURL);
	void barrage_url_extracted(QString);
	void barrage_extracted(QDomDocument);

	void finished();

private:
Q_SIGNALS:
	void cid_extracted(QString);
	void aid_extracted(QString);

private Q_SLOTS:

	void networkreplyed(QNetworkReply* reply)
	{
		reply->deleteLater();
	}

	void extract_aid_cid_stuff();

	void slot_aid_extracted(QString aid)
	{
		//barrage_url_extracted(QString::fromStdString));
	}

	void slot_cid_extracted(QString cid);

	void extract_flv_url();

	void slot_barrage_url_extracted(QString xml_url)
	{
		QNetworkReply * reply = current_reply = m_netmgr.get(QNetworkRequest(QUrl(xml_url)));

		connect(reply, SIGNAL(finished()), this, SLOT(extract_barrage()));
	}

	void extract_barrage()
	{
		QDomDocument m_xml;

		m_xml.setContent(current_reply->readAll());

		barrage_extracted(m_xml);
		finished();
	}

private:
	std::string m_bilibili_url;
	std::string cid, aid;

	QNetworkAccessManager m_netmgr;

	QNetworkReply* current_reply;
};
