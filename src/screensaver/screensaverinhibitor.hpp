

#pragma once

#include <string>
#include <cstdint>

#ifdef QT_DBUS_LIB

class ScreenSaverInhibitor
{
	ScreenSaverInhibitor(const ScreenSaverInhibitor&) = delete;
	ScreenSaverInhibitor& operator = (const ScreenSaverInhibitor&) = delete;

public:
	ScreenSaverInhibitor(const std::string& appname, const std::string& reason);
	~ScreenSaverInhibitor();

private:
	uint32_t m_cookie = 0;
};

#else
class ScreenSaverInhibitor
{
public:
	ScreenSaverInhibitor(const std::string& appname, const std::string& reason){}
};
#endif
