
#pragma once

#include <memory>
#include <QtCore>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QLabel>

#include <LinearMath/btAlignedObjectArray.h>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionShape;

class DanmuManager : public QObject
{
    Q_OBJECT

public:
	DanmuManager(QObject* parent = nullptr);
	~DanmuManager();

	void add_danmu(QGraphicsObject* danmuitem);

	void start();
protected Q_SLOTS:
	void iteration();

private:

	std::shared_ptr<btDefaultCollisionConfiguration> btCollisionConfiguration;
	std::shared_ptr<btCollisionDispatcher> m_CollisionDispatcher;
	std::shared_ptr<btBroadphaseInterface> m_overlappingPairCache;
	std::shared_ptr<btConstraintSolver> m_constrantsolver;
	std::shared_ptr<btDiscreteDynamicsWorld> m_world;


	btAlignedObjectArray<std::unique_ptr<btCollisionShape*>> m_collisonshapes;

	QThread m_sim_thread;
};

