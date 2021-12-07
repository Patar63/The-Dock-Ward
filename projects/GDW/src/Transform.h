#pragma once
//inclusions to help with drawing objects, model matrix, and the scene graph
#include "GLM/common.hpp"
#include "GLM/glm.hpp"
#include <vector>
//allow use of experimental glm features
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM/gtx/quaternion.hpp"
#include "GLM/gtx/transform.hpp"

class SMI_Transform
{
public:
	//constructors
	SMI_Transform();
	SMI_Transform(const SMI_Transform& other) = default;

	//This will update the transform on the object and all of its children.
	void DoFK();

	//This will recompute and return the global transform of this object.
	const glm::mat4& RecomputeGlobal();

	//This will return the current normal matrix of the object
	//(used for lighting). As above, make sure you have called
	//the appropriate update first.
	glm::mat3 GetNormal() const;

	//Sets a pointer to the parent object and updates child references
	//for the old and new parent objects accordingly.
	//Pass in nullptr if you wish for the object to not have a parent.
	void SetParent(SMI_Transform* parent);

	//functions for rotation
	void FixedRotate(glm::vec3 _rot);
	void RelativeRotate(glm::vec3 _rot);

	//setter functions
	void setPos(const glm::vec3 _Pos) { Pos = _Pos; RecomputeGlobal(); }
	void setScale(const glm::vec3 _Scale) { Scale = _Scale; RecomputeGlobal(); }
	void setRot(const glm::quat _Rot) { Rot = _Rot; RecomputeGlobal(); }
	void SetDegree(const glm::vec3 _Rot) { Rot = glm::quat(glm::radians(_Rot)); RecomputeGlobal(); }

	//getter functions
	glm::vec3 getPos() const { return Pos; }
	glm::vec3 getScale() const { return Scale; }
	glm::quat getRot() const { return Rot; }
	glm::mat4 getGlobal() const { return Global; }

	//destructor
	virtual ~SMI_Transform();

private:
	//mat4 variables for scene graph and model matrix
	glm::mat4 Local;
	glm::mat4 Global;

	//variables for position, rotation and scale
	glm::vec3 Pos;
	glm::vec3 Scale;
	glm::quat Rot;

protected:
	SMI_Transform* m_parent;
	std::vector<SMI_Transform*> m_children;

	//These functions are protected since they will be handled
	//by SetParent - we don't want to have to manually update this ourselves
	//whenever we switch an object's parent!
	void AddChild(SMI_Transform* child);
	void RemoveChild(SMI_Transform* child);
};