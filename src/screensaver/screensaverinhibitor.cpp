
#include <QtDBus>

#include "screensaverinhibitor.hpp"

#define Dbus_Service_ScreenSaver_Name "org.freedesktop.ScreenSaver"


ScreenSaverInhibitor::ScreenSaverInhibitor(const std::string& appname, const std::string& reason)
{
	if (QDBusConnection::sessionBus().isConnected())
	{
		QDBusInterface iface(Dbus_Service_ScreenSaver_Name, "/ScreenSaver", "org.freedesktop.ScreenSaver", QDBusConnection::sessionBus());

		QDBusReply<quint32> reply = iface.call("Inhibit", "bilibili_player", "playing video");

		if (reply.isValid())
		{
			m_cookie = reply.value();
		} 
	}
}

ScreenSaverInhibitor::~ScreenSaverInhibitor()
{
	if (QDBusConnection::sessionBus().isConnected() && m_cookie)
	{
		QDBusInterface iface(Dbus_Service_ScreenSaver_Name, "/ScreenSaver", "org.freedesktop.ScreenSaver", QDBusConnection::sessionBus());
		iface.call("UnInhibit", m_cookie);
	}
}

