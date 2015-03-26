
#pragma once

#include <QObject>
#include <QString>

class AssResPrivate;
class AssRes : public QObject
{
    Q_OBJECT

public:
	AssRes(QString assfile, QObject* parent = nullptr);

	virtual ~AssRes();

private:
    Q_DECLARE_PRIVATE(AssRes);
    AssResPrivate *d_ptr;

};


