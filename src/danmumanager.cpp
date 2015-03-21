
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
		auto itemsize = m_reperentor->boundingRect();
		m_cached.setOrigin(btVector3(qtrans.dx() + 2000, qtrans.dy(), itemsize.height() + 5));
	}

	virtual void getWorldTransform(btTransform& worldTrans) const
	{
		worldTrans = m_cached;
	}

	virtual void setWorldTransform(const btTransform& centerOfMassWorldTrans)
	{
		m_cached = centerOfMassWorldTrans;

		// 将物体绘制到 scene
		if(m_reperentor)
		{
			QTransform qtrans;

			auto dx = centerOfMassWorldTrans.getOrigin().getX();
			auto dy = centerOfMassWorldTrans.getOrigin().getY();

			qtrans.translate(dx, dy);


			qreal dz = centerOfMassWorldTrans.getOrigin().getZ();

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
	, m_CollisionDispatcher(&m_btCollisionConfiguration)
	, m_ground_shape(btVector3(0,0,-1),1)
	, m_ground_body(0, &m_ground_motion, &m_ground_shape)
	, m_world(&m_CollisionDispatcher, &m_overlappingPairCache, &m_constrantsolver, &m_btCollisionConfiguration)
{
	m_world.setGravity(btVector3(0.0, 0.0, 10));

	// 设定大地

	m_ground_body.setRestitution(0.00001);

	m_ground_body.setFriction(0);

	m_ground_body.setContactProcessingThreshold(4);

	m_world.addRigidBody(&m_ground_body);

	QTimer * ter = new QTimer(this);
	connect(ter, SIGNAL(timeout()), this, SLOT(iteration()));

	ter->setTimerType(Qt::PreciseTimer);

	m_sim_thread.start();

// 	ter->moveToThread(&m_sim_thread);
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
		m_world.stepSimulation(m_elapsedtimer.elapsed()/1000., 10);
	}

	m_elapsedtimer.restart();

	for(btRigidBody* budy : m_collisonshapes)
	{
		auto dx = budy->getCenterOfMassPosition().getX();

		if (budy->getLinearVelocity().getX() > - 100)
			budy->setDamping(0.1, 0);

		if (dx > video_width)
			continue;

		if (budy->getLinearVelocity().getX() > - 100)
			budy->applyCentralImpulse(btVector3(-100, 0, 0));
		else if (budy->getLinearVelocity().getX() < -250)
			budy->applyCentralImpulse(btVector3(200, 0, 0));
	}
}

void DanmuManager::add_danmu(QGraphicsObject* danmuitem)
{
	QRectF item_size = danmuitem->boundingRect();
	btScalar mass =item_size.height() * item_size.width();
	auto myMotionState = new danmuMotionState(danmuitem);

	auto colShape = new btCapsuleShape(item_size.height()/2, item_size.width() - item_size.height() );

	colShape->setLocalScaling(btVector3(1,1,10));

	btVector3 localInertia(0,0,0);
	colShape->calculateLocalInertia(1,localInertia);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(1, myMotionState, colShape, localInertia);

	btRigidBody * danmu_body = new btRigidBody(rbInfo);

	myMotionState->budy = danmu_body;

	danmu_body->setDamping(11, 0);

	// 好，加入 bullet 系统.
	m_world.addRigidBody(danmu_body);

	danmu_body->setLinearVelocity(btVector3(-80, 0, 80));

	danmu_body->setFriction(0);

	danmu_body->setAngularFactor(btVector3(0,0,0));

	danmu_body->applyCentralImpulse(btVector3(-4000, 0, 0));

	connect(danmuitem, &QObject::destroyed, danmuitem, [=](QObject *)
	{
		// 从世界删除
		m_world.removeRigidBody(danmu_body);
		m_collisonshapes.erase(std::find(m_collisonshapes.begin(), m_collisonshapes.end(), danmu_body));

		delete danmu_body;
		delete myMotionState;
		delete colShape;
	});

	m_collisonshapes.push_back(danmu_body);
}
