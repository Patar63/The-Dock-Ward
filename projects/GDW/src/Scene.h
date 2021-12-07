#pragma once
#include "entt.hpp"
#include "GLM/glm.hpp"
#include "GLM/common.hpp"
#include "Physics.h"
#include "Camera.h"
#include "Transform.h"
#include "Render.h"
#include <vector>

//class to create a scene 
class SMI_Scene
{
public:
	//constructor calls
	SMI_Scene();

	//copy, move, and assignment operators
	SMI_Scene(const SMI_Scene& oldScene) = default;
	SMI_Scene(SMI_Scene&&) = default;
	SMI_Scene& operator=(SMI_Scene&) = default;

	//destructor call
	~SMI_Scene();

	entt::entity CreateEntity();
	void DeleteEntity(entt::entity target);
	entt::registry& GetRegistry() { return Store; }

	//function declarations for a scene 
	virtual void InitScene();
	virtual void Update(float deltaTime);
	virtual void Render();
	virtual void PostRender();

	//Helper functions for entities 
	template <typename T>
	void Attach(entt::entity target);
	template <typename T>
	void AttachCopy(entt::entity target, const T& copy);
	template <typename T>
	T& GetComponent(entt::entity target);
	template <typename T>
	bool HasComponent(entt::entity target);
	template <typename T>
	void Remove(entt::entity target);

	//Physics for scenes
	//gravity setter and getter
	void setGravity(const glm::vec3& _gravity) { gravity = _gravity; }
	glm::vec3 getGravity() const { return gravity; }

	//setter and getter for active scene 
	void setActive(const bool& _isActive) { isActive = _isActive; }
	bool getActive() const { return isActive; }

	//setter and getter for the pause scene
	void setPause(const bool& _isPaused) { isPaused = _isPaused; }
	bool getPause() const { return isPaused; }

	//setter and getter for camera
	void setCamera(const Camera::Sptr& _cam) { camera = _cam; }
	Camera::Sptr getCamera() const { return camera; }

private:
	//create registry
	entt::registry Store;
	//bool to tell if a scene is active
	bool isActive;

	//pause screen boolean
	bool isPaused;

	//physics variables
	glm::vec3 gravity;

	//physics world properties
	btDefaultCollisionConfiguration* CollisionConfig;
	btCollisionDispatcher* Dispatcher;
	btBroadphaseInterface* OverlappingPairCache;
	btSequentialImpulseConstraintSolver* Solver;
	//physics world
	btDiscreteDynamicsWorld* physicsWorld;


	//manages collisions
	void CollisionManage();

protected:
	//handle used to reference camera object
	Camera::Sptr camera;
	//vector to hold all collisions to manage
	std::vector<SMI_Collision::sptr> Collisions;
};


//type T templated functions
template <typename T>
inline void SMI_Scene::Attach(entt::entity target)
{
	Store.emplace<T>(target);
}
template <>
inline void SMI_Scene::Attach<SMI_Physics>(entt::entity target)
{
	Store.emplace<SMI_Physics>(target);
	SMI_Physics& phys = GetComponent<SMI_Physics>(target);

	phys.setEntity(target);
	physicsWorld->addRigidBody(phys.getRigidBody());
	phys.setInWorld(true);
}

template <typename T>
inline void SMI_Scene::AttachCopy(entt::entity target, const T& copy)
{
	Store.emplace_or_replace<T>(target, copy);
}
template <>
inline void SMI_Scene::AttachCopy<SMI_Physics>(entt::entity target, const SMI_Physics& copy)
{
	Store.emplace_or_replace<SMI_Physics>(target, copy);
	SMI_Physics& phys = GetComponent<SMI_Physics>(target);

	phys.setEntity(target);
	physicsWorld->addRigidBody(phys.getRigidBody());
	phys.setInWorld(true);
}

template <typename T>
inline T& SMI_Scene::GetComponent(entt::entity target)
{
	return Store.get<T>(target);
}
template <typename T>
inline bool SMI_Scene::HasComponent(entt::entity target)
{
	return Store.has<T>(target);
}

template <typename T>
inline void SMI_Scene::Remove(entt::entity target)
{
	//deletes component
	Store.remove<T>(target);
}
template <>
inline void SMI_Scene::Remove<SMI_Physics>(entt::entity target)
{
	//deletes bullet components and physics
	btRigidBody* TargetBody = Store.get<SMI_Physics>(target).getRigidBody();
	delete TargetBody->getMotionState();
	delete TargetBody->getCollisionShape();
	physicsWorld->removeRigidBody(TargetBody);
	delete TargetBody;

	//deletes component
	Store.remove<SMI_Physics>(target);
}