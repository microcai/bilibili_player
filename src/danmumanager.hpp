
#pragma once

#include <memory>
#include <QtCore>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QLabel>
#include <QElapsedTimer>

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Character/btCharacterControllerInterface.h>

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

	btStaticPlaneShape m_ground_shape;
	btDefaultMotionState m_ground_motion;
	btRigidBody m_ground_body;

	std::vector<btRigidBody*> m_collisonshapes;

	QThread m_sim_thread;

	QElapsedTimer m_elapsedtimer;
public:
	int video_width;
};

