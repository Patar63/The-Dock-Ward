#pragma once
#include <memory>
#include <unordered_map>
#include "Uniform.h"
#include "ITexture.h"

class SMI_Material
{
public:
	typedef std::shared_ptr<SMI_Material> Sptr;

	  inline static Sptr Create() {
		  return std::make_shared<SMI_Material>();
	  }

public:
	//constructor
	SMI_Material();

	//functions
	//set all uniforms
	void BindAllUniform();

	//set all textures
	void BindAllTextures();
	void UnbindAllTextures();

	//setters
	void setShader(const Shader::Sptr& _shader) { m_Shader = _shader; }

	//creates uniform objects elsewhere, pass into SetUniform, then add to material
	void setUniform(const Uniform::Sptr& _uniform);

	void setTexture(const ITexture::Sptr& _texture, const int& slot);

	//getters
	Shader::Sptr getShader() const { return m_Shader; }
	Uniform::Sptr getUniform(const std::string& UniformName);
	ITexture::Sptr getTexture(const int& TextureSlot);

	//destructor
	~SMI_Material();

private:
	Shader::Sptr m_Shader;
	//holds an unordered map of our uniforms
	std::unordered_map<std::string, Uniform::Sptr> m_UniformMap;
	//holds an unordered map of all textures
	std::unordered_map<int, ITexture::Sptr> m_TextureMap;

};