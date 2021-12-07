#pragma once
#include "GLM/glm.hpp"
#include "entt.hpp"
#include "btBulletDynamicsCommon.h"

enum class SMI_PhysicsBodyType
{
	STATIC = 0,
	KINEMATIC = 1,
	DYNAMIC = 2
};

class SMI_Physics
{
public:
	SMI_Physics();
	SMI_Physics(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, entt::entity _Entity, 
									SMI_PhysicsBodyType _BodyType = SMI_PhysicsBodyType::DYNAMIC, float _objMass = 1.f);

	//copy, move, and assingment constructors for entt
	SMI_Physics(const SMI_Physics&) = default;
	SMI_Physics(SMI_Physics&&) = default;
	SMI_Physics& operator=(SMI_Physics&) = default;

	~SMI_Physics();

	void Update(float deltaTime);

	//getters and setters for any physics bodies
	SMI_PhysicsBodyType getBodyType() const { return BodyType; }

	float getmass() const { return objMass; }

	void setInWorld(const bool& _inWorld) { inWorld = _inWorld; }
	bool getInWorld() const { return inWorld; }

	void setHasGravity(const bool& _hasGravity) { hasGravity = _hasGravity; }
	bool getHasGravity() const { return hasGravity; }

	void setEntity(const entt::entity& _Entity) { Entity = _Entity; }
	entt::entity getEntity() const { return Entity; }

	btRigidBody* getRigidBody() const { return objRigidBody; }

	//Functions to interface with Bullet
	void SetPosition(glm::vec3 pos);
	glm::vec3 GetPosition();
	void SetRotation(glm::vec3 rot);
	glm::vec3 GetRotation();
	
	void AddForce(glm::vec3 force);
	void AddImpulse(glm::vec3 impulse);
	void ClearForces();

private:
	//variables used for physics collision and motion
	float objMass;
	btCollisionShape* objShape;
	btTransform objPos;
	btMotionState* objMotionState;
	btRigidBody* objRigidBody;

	bool inWorld;
	bool hasGravity;

	entt::entity Entity;

	SMI_PhysicsBodyType BodyType;
};

class SMI_Collision
{
public:
	typedef std::shared_ptr<SMI_Collision>sptr;

	static inline sptr Create()
	{
		return std::make_shared<SMI_Collision>();
	}

	//Removing implicit constructors
	SMI_Collision(const SMI_Collision& other) = delete;
	SMI_Collision(SMI_Collision& other) = delete;
	SMI_Collision& operator=(const SMI_Collision& other) = delete;
	SMI_Collision& operator=(SMI_Collision&& other) = delete;


	//constructors and destructors
	SMI_Collision();
	~SMI_Collision() = default;

	//getters and setters
	void setB1(const entt::entity& _b1) { b1 = _b1; }
	entt::entity getB1() const { return b1; }
	void setB2(const entt::entity& _b2) { b2 = _b2; }
	entt::entity getB2() const { return b2; }

	//Checks collisions to see if they're caused by the same two objects (no repeats collisions)
	static bool Same(sptr C1, sptr C2) { return ((C1->b1 == C2->b1 && C1->b2 == C2->b2) || (C1->b1 == C2->b2 && C1->b2 == C2->b1)); }

private:
	//used to define the space and two objects involved in a collision
	entt::entity b1;
	entt::entity b2;
};
