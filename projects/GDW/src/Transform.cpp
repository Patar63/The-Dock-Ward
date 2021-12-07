#include "Transform.h"

SMI_Transform::SMI_Transform()
{
	m_parent = nullptr;

	Pos = glm::vec3(0.0f);
	Scale = glm::vec3(1.0f);
	Rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	Global = glm::mat4(1.0f);
}

void SMI_Transform::DoFK()
{
	//First, grab our local transform...
	glm::mat4 local  =  glm::translate(Pos) *
						glm::toMat4(glm::normalize(Rot)) *
						glm::scale(Scale);

	//If we have a parent, we need to multiply by our parent's
	//global transform.
	if (m_parent != nullptr)
		Global = m_parent->Global * local;

	//If we have no parent object, our global transform is our
	//local transform!
	else
		Global = local;

	//FK is recursive - we now repeat this process on our child nodes.
	//Eventually, we'll be at the bottom of the hierarchy and this will
	//return.
	for (auto* child : m_children)
	{
		child->DoFK();
	}
}

const glm::mat4& SMI_Transform::RecomputeGlobal()
{
	//Just as with FK, compute our local, then multiply with
		//our parent's transform if applicable.

	glm::mat4 local  =  glm::translate(Pos) *
						glm::toMat4(Rot) *
						glm::scale(Scale);

	if (m_parent != nullptr)
		Global = m_parent->RecomputeGlobal() * local;
	else
		Global = local;

	return Global;
}

glm::mat3 SMI_Transform::GetNormal() const
{
	//The normal matrix is used to transform the normals of our mesh
		//for correct lighting.
		//Basically, we need to orient the normals and undo any non-uniform scaling
		//to prevent strange artifacts (since normals are just directions.)
		//If we're using a uniform scale, then we can just pass the top 3x3 of our
		//transform matrix (the rotation/scale bit) - since we'll re-normalize
		//the normals in our shader anyways.
	if (Scale.x == Scale.y && Scale.x == Scale.z)
		return glm::mat3(Global);

	//If we do have a non-uniform scale, then we need to undo that scale, 
	//hence the inverse. However, we want to preserve our rotation.
	//Since the inverse of a rotation matrix IS its transpose, by adding
	//in the transpose we can effectively spit our rotation matrix with
	//the inverse of our scale applied.
	//You could also do some trickery here with the reciprocal of your
	//scale vector and your rotation quaternion, but this is a bit more
	//"bulletproof" and straightforward if you're doing oddball transformations.
	return glm::inverse(glm::transpose(glm::mat3(Global)));
}

void SMI_Transform::SetParent(SMI_Transform* parent)
{
}

void SMI_Transform::FixedRotate(glm::vec3 _rot)
{
	Rot = glm::quat(glm::radians(_rot)) * Rot;
	RecomputeGlobal();
}

void SMI_Transform::RelativeRotate(glm::vec3 _rot)
{
	Rot = Rot * glm::quat(glm::radians(_rot));
	RecomputeGlobal();
}

SMI_Transform::~SMI_Transform()
{
	SetParent(nullptr);
}

void SMI_Transform::AddChild(SMI_Transform* child)
{
}

void SMI_Transform::RemoveChild(SMI_Transform* child)
{
}
