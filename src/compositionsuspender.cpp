
#include "compositionsuspender.hpp"

#ifdef HAVE_KF5_WINDOWSYSTEM
#include <KWindowSystem>
#endif

CompositionSuspender::CompositionSuspender(QWidget* parent)
	: QObject(parent)
{
	bool is_full = parent->isFullScreen();
#ifdef HAVE_KF5_WINDOWSYSTEM
	KWindowSystem::setBlockingCompositing(parent->winId(), is_full);
#endif
}

CompositionSuspender::~CompositionSuspender()
{
#ifdef HAVE_KF5_WINDOWSYSTEM
	KWindowSystem::setBlockingCompositing(qobject_cast< const QWidget* >(parent())->winId(),false);
#endif
}
