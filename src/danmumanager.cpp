
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
	}

	virtual void getWorldTransform(btTransform& worldTrans) const
	{
		worldTrans.setIdentity();
		QTransform qtrans = m_reperentor->transform();

		worldTrans.setOrigin(btVector3(qtrans.dx(), qtrans.dy(), -100));
	}

	virtual void setWorldTransform(const btTransform& centerOfMassWorldTrans)
	{
// 		btDefaultMotionState::setWorldTransform(centerOfMassWorldTrans);

		// 将物体绘制到 scene

		QTransform qtrans;

		auto dx = centerOfMassWorldTrans.getOrigin().getX();
		auto dy = centerOfMassWorldTrans.getOrigin().getY();

		qtrans.translate(dx, dy);

		m_reperentor->setTransform(qtrans);
	}

protected:
	QGraphicsObject* m_reperentor;
};

DanmuManager::DanmuManager(QObject* parent)
	: QObject(parent)
{
	btCollisionConfiguration.reset(new btDefaultCollisionConfiguration);
	m_CollisionDispatcher.reset(new btCollisionDispatcher(btCollisionConfiguration.get()));

	m_overlappingPairCache.reset(new btDbvtBroadphase);

	m_constrantsolver.reset(new btSequentialImpulseConstraintSolver);

	m_world.reset(new btDiscreteDynamicsWorld(m_CollisionDispatcher.get(), m_overlappingPairCache.get(), m_constrantsolver.get(), btCollisionConfiguration.get()));

// 	m_world->setGravity(btVector3(0.0, 0.0, 9.8));
	m_world->setGravity(btVector3(-80, 0.0, 0.0));


	QTimer * ter = new QTimer(this);
	connect(ter, SIGNAL(timeout()), this, SLOT(iteration()));

	// 	ter->moveToThread(&m_sim_thread);
	//  50fps 已经很流畅了
	ter->setInterval(1000/50);
	ter->start();
}

DanmuManager::~DanmuManager()
{
}

void DanmuManager::iteration()
{
	QTimer * sender_timer = qobject_cast<QTimer*>(sender());

	if (sender_timer)
	{
		m_world->stepSimulation(sender_timer->interval() / 1000., 1);
	}
}

void DanmuManager::add_danmu(QGraphicsObject* danmuitem)
{
	QRectF item_size = danmuitem->boundingRect();
	btScalar mass =item_size.height() * item_size.width();
	auto myMotionState = new danmuMotionState(danmuitem);

	auto colShape = new btCapsuleShape(item_size.height()/2, item_size.width());

	btVector3 localInertia(0,0,0);
	colShape->calculateLocalInertia(mass,localInertia);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody * danmu_body = new btRigidBody(rbInfo);
	danmuitem->setProperty("bullet_body", QVariant::fromValue<btRigidBody*>(danmu_body));

	// 好，加入 bullet 系统.

	m_world->addRigidBody(danmu_body);

	connect(danmuitem, &QObject::destroyed, danmuitem, [=](QObject *){
		// 从世界删除
		delete danmu_body;
		delete myMotionState;
		delete colShape;
	});
}
