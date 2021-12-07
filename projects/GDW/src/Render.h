#pragma once
#include "Material.h"
#include "VertexArrayObject.h"

class Renderer
{
public:
	//constructors
	Renderer();
	Renderer(SMI_Material::Sptr _mat, VertexArrayObject::Sptr _vao);

	//functions
	void Render();

	//setters
	void setMaterial(SMI_Material::Sptr _material) { m_Material = _material; }
	void setVAO(VertexArrayObject::Sptr _vao) { m_VAO = _vao; }

	//getters
	SMI_Material::Sptr getMaterial() const { return m_Material; }
	VertexArrayObject::Sptr getVAO() const { return m_VAO; }

	//destructor
	~Renderer();

private:
	SMI_Material::Sptr m_Material;
	VertexArrayObject::Sptr m_VAO;

};