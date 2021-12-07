#include "ObjLoader.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

// Borrowed from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
#pragma region String Trimming

// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

#pragma endregion 

VertexArrayObject::Sptr ObjLoader::LoadFromFile(const std::string& filename)
{
	// Open our file in binary mode
	std::ifstream file;
	file.open(filename, std::ios::binary);

	// If our file fails to open, we will throw an error
	if (!file) {
		throw std::runtime_error("Failed to open file");
	}

	std::string line;
	
	// TODO: Load data from file
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> nrml;
	std::vector<glm::vec3> vertecies;

	glm::vec3 vecData;
	glm::ivec3 vertexIndicies;

	//read abd process whole file 
	while (file.peek() != EOF)
	{
		std::string command;
		file >> command;

		if (command == "#")
		{
			std::getline(file, line);
		}
		else if (command == "vt")
		{
			glm::vec2 temp;

			file >> temp.x >> temp.y;
			uvs.push_back(temp);
		}
		else if (command == "vn")
		{
			file >> vecData.x >> vecData.y >> vecData.z;
			nrml.push_back(vecData);
		}
		else if (command == "v")
		{
			file >> vecData.x >> vecData.y >> vecData.z;
			positions.push_back(vecData);
		}
		else if (command == "f")
		{
			std::getline(file, line);
			trim(line);
			std::stringstream stream = std::stringstream(line);

			for (int i = 0; i < 3; i++)
			{
				char separator;
				stream >> vertexIndicies.x >> separator >> vertexIndicies.y >> separator >> vertexIndicies.z;

				vertexIndicies -= glm::ivec3(1);

				vertecies.push_back(vertexIndicies);
			}
		}
	}

	// TODO: Generate mesh from the data we loaded
	std::vector<VertexPosNormTexCol> vertexData;

	for (int i = 0; i < vertecies.size(); i++)
	{
		glm::ivec3 attribs = vertecies[i];
		
		// Extract attributes from lists (except color)
		glm::vec3 position = positions[attribs.x];
		glm::vec3 normal = nrml[attribs.z];
		glm::vec2 uv = uvs[attribs.y];
		glm::vec4 color = glm::vec4(1.0f);
		
		// Add the vertex to the mesh    
		vertexData.push_back(VertexPosNormTexCol(position, normal, uv, color));
	}

	// Create a vertex buffer and load all our vertex data
	VertexBuffer::Sptr vertexBuffer = VertexBuffer::Create();
	vertexBuffer->LoadData(vertexData.data(), vertexData.size());
	
	// Create the VAO, and add the vertices
	VertexArrayObject::Sptr result = VertexArrayObject::Create();
	result->AddVertexBuffer(vertexBuffer, VertexPosNormTexCol::V_DECL);

	return result;
	//return VertexArrayObject::Create();
}
