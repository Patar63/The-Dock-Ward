#define GLM_ENABLE_EXPERIMENTAL
#include "GLM/gtx/quaternion.hpp"
#include "Physics.h"

SMI_Physics::SMI_Physics()
{
    //set up bullet collision shape
    objShape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
    //set up bullet transform
    objPos.setIdentity();
    objPos.setOrigin(btVector3(0.f, 0.f, 0.f));
    objPos.setRotation(btQuaternion());
    //setup up bullet motion state
    objMotionState = new btDefaultMotionState(objPos);

    //setup mass, static v dynmaic status, and local internia
    btVector3 localIntertia(0, 0, 0);
    objMass = 1.0f;

    BodyType = SMI_PhysicsBodyType::DYNAMIC;

    //create the rigidbody
    btRigidBody::btRigidBodyConstructionInfo rbInfo(objMass, objMotionState, objShape, localIntertia);
    objRigidBody = new btRigidBody(rbInfo);

    objRigidBody->setActivationState(DISABLE_DEACTIVATION);

    hasGravity = true;

    inWorld = false;

    Entity = static_cast<entt::entity>(-1);

    objRigidBody->setUserPointer(reinterpret_cast<void*>(static_cast<uint32_t>(Entity)));
}

SMI_Physics::SMI_Physics(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, entt::entity _Entity, SMI_PhysicsBodyType _BodyType, float _objMass)
{ 
    //set up bullet collision shape
    objShape = new btBoxShape(btVector3(scale.x /2, scale.y / 2, scale.z / 2));
    //set up bullet transform
    objPos.setIdentity();
    objPos.setOrigin(btVector3(position.x, position.y, position.z));
    glm::quat rot = glm::quat(glm::radians(rotation));
    objPos.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
    //setup up bullet motion state
    objMotionState = new btDefaultMotionState(objPos);

    //setup mass, static v dynmaic status, and local internia
    btVector3 localIntertia(0, 0, 0);
    objMass = _objMass;

    BodyType = _BodyType;
    if (BodyType == SMI_PhysicsBodyType::STATIC || BodyType == SMI_PhysicsBodyType::KINEMATIC)
    {
        _objMass = 0;
    }

    //create the rigidbody
    btRigidBody::btRigidBodyConstructionInfo rbInfo(objMass, objMotionState, objShape, localIntertia);
    objRigidBody = new btRigidBody(rbInfo);

    //if it's kinematic, set the kinematic flag
    if (BodyType == SMI_PhysicsBodyType::KINEMATIC) {
        objRigidBody->setCollisionFlags(objRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    }
    else if (BodyType == SMI_PhysicsBodyType::STATIC) {
        objRigidBody->setCollisionFlags(objRigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    }

    objRigidBody->setActivationState(DISABLE_DEACTIVATION);

    hasGravity = true;

    inWorld = false;

    Entity = _Entity;

    objRigidBody->setUserPointer(reinterpret_cast<void*>(static_cast<uint32_t>(Entity)));
}

SMI_Physics::~SMI_Physics()
{
}

void SMI_Physics::Update(float deltaTime)
{
}

void SMI_Physics::SetPosition(glm::vec3 pos)
{
    btVector3 newPos = btVector3(pos.x, pos.y, pos.z);
    btTransform TransPos;
    objRigidBody->getMotionState()->getWorldTransform(TransPos);
    TransPos.setOrigin(newPos);
    objRigidBody->getMotionState()->setWorldTransform(TransPos);
}

glm::vec3 SMI_Physics::GetPosition()
{
    btTransform TransPos;
    objRigidBody->getMotionState()->getWorldTransform(TransPos);
    btVector3 pos = TransPos.getOrigin();

    return glm::vec3((float)pos.getX(), (float)pos.getY(), (float)pos.getZ());
}

void SMI_Physics::SetRotation(glm::vec3 rot)
{
}

glm::vec3 SMI_Physics::GetRotation()
{
	return glm::vec3();
}

void SMI_Physics::AddForce(glm::vec3 force)
{
    objRigidBody->applyCentralForce(btVector3(force.x, force.y, force.z));
}

void SMI_Physics::AddImpulse(glm::vec3 impulse)
{
    objRigidBody->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}

void SMI_Physics::ClearForces()
{
    objRigidBody->clearForces();
}

SMI_Collision::SMI_Collision()
{
    b1 = entt::null;
    b2 = entt::null;
}
