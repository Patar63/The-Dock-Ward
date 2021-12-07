#include "Material.h"

SMI_Material::SMI_Material()
{
}

void SMI_Material::BindAllUniform()
{
	std::unordered_map<std::string, Uniform::Sptr>::iterator it = m_UniformMap.begin();

	while (it != m_UniformMap.end())
	{
		it->second->SetUniform(m_Shader);
		it->second->SetUniformMatrix(m_Shader);
		it++;
	}
}

void SMI_Material::BindAllTextures()
{
	std::unordered_map<int, ITexture::Sptr>::iterator it = m_TextureMap.begin();

	while (it != m_TextureMap.end())
	{
		it->second->Bind(it->first);
		it++;
	}
}

void SMI_Material::UnbindAllTextures()
{
	std::unordered_map<int, ITexture::Sptr>::iterator it = m_TextureMap.begin();

	while (it != m_TextureMap.end())
	{
		it->second->Unbind(it->first);
		it++;
	}
}

void SMI_Material::setUniform(const Uniform::Sptr& _uniform)
{
	std::string Name = _uniform->getName();
	m_UniformMap[Name] = _uniform;
}

void SMI_Material::setTexture(const ITexture::Sptr& _texture, const int& slot)
{
	m_TextureMap[slot] = _texture;
}

Uniform::Sptr SMI_Material::getUniform(const std::string& UniformName)
{
	if (m_UniformMap.find(UniformName) != m_UniformMap.end())
	{
		return m_UniformMap.at(UniformName);
	}
	
	return nullptr;
}

ITexture::Sptr SMI_Material::getTexture(const int& TextureSlot)
{
	if (m_TextureMap.find(TextureSlot) != m_TextureMap.end())
	{
		return m_TextureMap.at(TextureSlot);
	}

	return nullptr;
}

SMI_Material::~SMI_Material()
{
}
