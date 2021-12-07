#pragma once
#include "Shader.h"
#include "GLM/glm.hpp"
#include <memory>
#include <string>

class Uniform
{
public:
	typedef std::shared_ptr<Uniform> Sptr;

public:
	//pure virtual function. Acts as a parent for UniformObject
	virtual void SetUniform(Shader::Sptr) = 0;

	virtual void SetUniformMatrix(Shader::Sptr) = 0;

	//setters
	void setName(const std::string& _name) { UniformName = _name; }
	
	//getters
	std::string getName() const { return UniformName; }

protected:
	std::string UniformName;
};


template <typename T>
class UniformObject : public Uniform
{
public:
	typedef std::shared_ptr<UniformObject> Sptr;

	inline static Sptr Create() {
		return std::make_shared<UniformObject>();
	}

public:
	//declare function
	void SetUniform(Shader::Sptr);
	void SetUniformMatrix(Shader::Sptr) {};

	//setters
	void setData(const T& _data) { UniformData = _data; }

	//getters
	T getData() const { return UniformData; }

protected:
	//T variable to represent the Uniform. Passed to shader
	T UniformData;
};

template <typename T>
class UniformMatrixObject : public Uniform
{
public:
	typedef std::shared_ptr<UniformMatrixObject> Sptr;

	inline static Sptr Create() {
		return std::make_shared<UniformMatrixObject>();
	}

public:
	//declare function
	void SetUniform(Shader::Sptr) {};
	void SetUniformMatrix(Shader::Sptr);

	//setters
	void setData(const T& _data) { UniformData = _data; }

	//getters
	T getData() const { return UniformData; }

protected:
	//T variable to represent the Uniform. Passed to shader
	T UniformData;
};


//function to set the Uniform
template<typename T>
inline void UniformObject<T>::SetUniform(Shader::Sptr shader)
{
	shader->SetUniform(UniformName, UniformData);
}

template<typename T>
inline void UniformMatrixObject<T>::SetUniformMatrix(Shader::Sptr shader)
{
	shader->SetUniformMatrix(UniformName, UniformData);
}
