
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
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>

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

	btDefaultCollisionConfiguration m_btCollisionConfiguration;
	btCollisionDispatcher m_CollisionDispatcher;
	btDbvtBroadphase m_overlappingPairCache;
	btSequentialImpulseConstraintSolver m_constrantsolver;
	btDiscreteDynamicsWorld m_world;

	btStaticPlaneShape m_ground_shape;
	btDefaultMotionState m_ground_motion;
	btRigidBody m_ground_body;

	std::vector<btRigidBody*> m_collisonshapes;

	QThread m_sim_thread;

	QElapsedTimer m_elapsedtimer;
public:
	int video_width;
};

