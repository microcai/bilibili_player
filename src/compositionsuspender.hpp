
#pragma once

#include <QtCore>
#include <QWidget>

class CompositionSuspender : public QObject
{
	Q_OBJECT

public:
	CompositionSuspender(QWidget* parent = nullptr);
	~CompositionSuspender();
};
