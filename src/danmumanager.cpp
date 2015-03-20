
#include "danmumanager.hpp"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include <QGraphicsProxyWidget>

Q_DECLARE_METATYPE(btRigidBody*)

struct danmuMotionState : public btMotionState
{
	danmuMotionState(QGraphicsObject* widget)
		: m_reperentor(widget)
	{
		m_cached.setIdentity();
		QTransform qtrans = m_reperentor->transform();
		m_cached.setOrigin(btVector3(qtrans.dx() + 2000, qtrans.dy(), -18));
	}

	virtual void getWorldTransform(btTransform& worldTrans) const
	{
		worldTrans = m_cached;
	}

	virtual void setWorldTransform(const btTransform& centerOfMassWorldTrans)
	{
		m_cached = centerOfMassWorldTrans;

		if (budy->getLinearVelocity().getX() > - 50)
			budy->applyCentralImpulse(btVector3(-100, 0, 0));


		// 将物体绘制到 scene
		if(m_reperentor)
		{
			QTransform qtrans;

			auto dx = centerOfMassWorldTrans.getOrigin().getX();
			auto dy = centerOfMassWorldTrans.getOrigin().getY();

			if (budy->getLinearVelocity().getX() < -200 && dx < 1280*2)
				budy->applyDamping(1);


			qtrans.translate(dx, dy);

// 			qtrans.rotateRadians(centerOfMassWorldTrans.getRotation().getAngle());

			qreal dz = centerOfMassWorldTrans.getOrigin().getZ();

			qreal scale;


			scale = (( dz - 0 ) / -100);

			if ( scale < 0.3 )
				scale = 0.3;
			if (scale > 1.5)
				scale = 1.5;

//  			qtrans.scale(scale, scale);

			m_reperentor->setTransform(qtrans);

			if (dx < -100)
			{
				m_reperentor->deleteLater();
				m_reperentor.clear();
			}
		}

	}

protected:
	QPointer<QGraphicsObject> m_reperentor;

	btTransform m_cached;

public:
	btRigidBody* budy;
};

DanmuManager::DanmuManager(QObject* parent)
	: QObject(parent)
	, m_ground_shape(btVector3(0,0,-1),1)
	, m_ground_body(0, &m_ground_motion, &m_ground_shape)
{
	btCollisionConfiguration.reset(new btDefaultCollisionConfiguration);
	m_CollisionDispatcher.reset(new btCollisionDispatcher(btCollisionConfiguration.get()));

	m_overlappingPairCache.reset(new btDbvtBroadphase);

	m_constrantsolver.reset(new btSequentialImpulseConstraintSolver);

	m_world.reset(new btDiscreteDynamicsWorld(m_CollisionDispatcher.get(), m_overlappingPairCache.get(), m_constrantsolver.get(), btCollisionConfiguration.get()));

	m_world->setGravity(btVector3(0.0, 0.0, 10));

	// 设定大地

	m_ground_body.setRestitution(0.00001);

	m_ground_body.setFriction(0);

	m_ground_body.setContactProcessingThreshold(4);

	m_world->addRigidBody(&m_ground_body);

	QTimer * ter = new QTimer(this);
	connect(ter, SIGNAL(timeout()), this, SLOT(iteration()));

	ter->setTimerType(Qt::PreciseTimer);

	m_sim_thread.start();

	ter->moveToThread(&m_sim_thread);
	//  50fps 已经很流畅了
	ter->setInterval(1000/50);
	ter->start();
	m_elapsedtimer.start();
}

DanmuManager::~DanmuManager()
{
	m_sim_thread.quit();
	m_sim_thread.wait();
}

void DanmuManager::iteration()
{
	QTimer * sender_timer = qobject_cast<QTimer*>(sender());

	if (sender_timer)
	{
		m_world->stepSimulation(m_elapsedtimer.elapsed()/1000., 10);
	}

	m_elapsedtimer.restart();
}

void DanmuManager::add_danmu(QGraphicsObject* danmuitem)
{
	QRectF item_size = danmuitem->boundingRect();
	btScalar mass =item_size.height() * item_size.width();
	auto myMotionState = new danmuMotionState(danmuitem);

	auto colShape = new btCapsuleShape(item_size.height()/2, item_size.width());

	colShape->setLocalScaling(btVector3(1,1,0.3));

	btVector3 localInertia(0,0,0);
	colShape->calculateLocalInertia(mass,localInertia);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(1, myMotionState, colShape, localInertia);

	btRigidBody * danmu_body = new btRigidBody(rbInfo);

	myMotionState->budy = danmu_body;

	danmu_body->setDamping(0.1, 0);

	// 好，加入 bullet 系统.
	m_world->addRigidBody(danmu_body);

	danmu_body->setLinearVelocity(btVector3(-80, 0, 80));

	danmu_body->setFriction(0);

	danmu_body->setAngularFactor(btVector3(0,0,0));

	danmu_body->applyCentralImpulse(btVector3(-8000, 0, 0));

	connect(danmuitem, &QObject::destroyed, danmuitem, [=](QObject *){
		// 从世界删除
		m_world->removeRigidBody(danmu_body);

		delete danmu_body;
		delete myMotionState;
		delete colShape;
	});
}
