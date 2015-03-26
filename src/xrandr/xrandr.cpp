
#include <memory>
#include <QX11Info>
#include <QDebug>
#include <X11/X.h>
#include <X11/extensions/Xrandr.h>

#include "xrandr.hpp"

QSize native_res_for_monitior()
{
	Display* dpy = QX11Info::display();
	int major_version_return;
	int minor_version_return;
	XRRQueryVersion(dpy, &major_version_return, &minor_version_return);

	int event_base_return;
	int error_base_return;

	if (!XRRQueryExtension(dpy, &event_base_return, &error_base_return))
	{
		return QSize();
	}

	qDebug() << "XRandR extension supported";

	int nscreen_sizes = 0;

	XRRScreenSize * screen_sizes;
	screen_sizes = XRRSizes(dpy, QX11Info::appScreen(), &nscreen_sizes);

	for ( int i = 0 ; i < nscreen_sizes; i++ )
	{
		XRRScreenSize ss = screen_sizes[i];
		QSize qss(ss.width, ss.height);
		return qss;
	}

	return QSize();
}
