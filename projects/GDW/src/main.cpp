#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <Logging.h>

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArrayObject.h"
#include "Shader.h"
#include "Camera.h"
#include "Player.h"
#include "Physics.h"
#include "Scene.h"
#include "Texture2D.h"
#include "TextureCube.h"

#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "VertexTypes.h"

#include <memory>
#include <filesystem>
#include <json.hpp>
#include <fstream>
#include <string>
#include <iostream>

#define LOG_GL_NOTIFICATIONS

/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
	case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
	case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
	case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
	case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
#ifdef LOG_GL_NOTIFICATIONS
	case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
#endif
	default: break;
	}
}

// Stores our GLFW window in a global variable for now
GLFWwindow* window;
// The current size of our window in pixels
glm::ivec2 windowSize = glm::ivec2(1500, 1000);
// The title of our GLFW window
std::string windowTitle = "Project Dock-Ward";





void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowSize = glm::ivec2(width, height);
}





/// <summary>
/// Handles intializing GLFW, should be called before initGLAD, but after Logger::Init()
/// Also handles creating the GLFW window
/// </summary>
/// <returns>True if GLFW was initialized, false if otherwise</returns>
bool initGLFW() {
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	//Create a new GLFW window and make it current
	window = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set our window resized callback
	glfwSetWindowSizeCallback(window, GlfwWindowResizedCallback);

	return true;
}

/// <summary>
/// Handles initializing GLAD and preparing our GLFW window for OpenGL calls
/// </summary>
/// <returns>True if GLAD is loaded, false if there was an error</returns>
bool initGLAD() {
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		LOG_ERROR("Failed to initialize Glad");
		return false;
	}
	return true;
}


GLuint shader_program;

//load in vertex and fragment shaders
bool loadShaders()
{
	// Read Shaders from file
	std::string vert_shader_str;
	std::ifstream vs_stream("vertex_shader.glsl", std::ios::in);
	if (vs_stream.is_open())
	{
		std::string Line = "";
		while (getline(vs_stream, Line))
			vert_shader_str += "\n" + Line;
		vs_stream.close();
	}
	else
	{
		printf("Could not open vertex shader!!\n");
		return false;
	}
	const char* vs_str = vert_shader_str.c_str();

	std::string frag_shader_str;
	std::ifstream fs_stream("frag_shader.glsl", std::ios::in);
	if (fs_stream.is_open()) {
		std::string Line = "";
		while (getline(fs_stream, Line))
			frag_shader_str += "\n" + Line;
		fs_stream.close();
	}
	else {
		printf("Could not open fragment shader!!\n");
		return false;
	}
	const char* fs_str = frag_shader_str.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_str, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_str, NULL);
	glCompileShader(fs);

	shader_program = glCreateProgram();
	glAttachShader(shader_program, fs);
	glAttachShader(shader_program, vs);
	glLinkProgram(shader_program);

	return true;
}

template <typename T>
//templated LERP to be used for bullets
T Lerp(T a, T b, float t)
{
	return(1.0f - t) * a + b * t;
}
float tran2;
void keyboard()
{
	float camera;

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		tran2 -= 0.06f;
	}
}

class GameScene : public SMI_Scene
{
public:
	GameScene() { SMI_Scene(); }

	void InitScene()
	{
		SMI_Scene::InitScene();

		// Load our shaders
		Shader::Sptr shader = Shader::Create();
		shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", ShaderPartType::Vertex);
		shader->LoadShaderPartFromFile("shaders/frag_shader.glsl", ShaderPartType::Fragment);
		shader->Link();

		// GL states, we'll enable depth testing and backface fulling
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glClearColor(0.2f, 0.2f, 0.5f, 1.0f);

		// Get uniform location for the model view projection
		Camera::Sptr camera = Camera::Create();

		//camera position
		camera->SetPosition(glm::vec3(tran2, 10.5, 9.9));
		//this defines the point the camera is looking at
		camera->LookAt(glm::vec3(tran2));

		//camera->SetOrthoVerticalScale(5);
		setCamera(camera);

		//creates object
		VertexArrayObject::Sptr vao4 = ObjLoader::LoadFromFile("Models/window1.obj");
		{
			barrel = CreateEntity();

			//create texture
			Texture2D::Sptr window1Texture = Texture2D::Create("Textures/BrownTex.1001.png");
			//material
			SMI_Material::Sptr BarrelMat = SMI_Material::Create();
			BarrelMat->setShader(shader);

			//set textures
			BarrelMat->setTexture(window1Texture, 0);
			//render
			Renderer BarrelRend = Renderer(BarrelMat, vao4);
			AttachCopy(barrel, BarrelRend);
			//transform
			SMI_Transform BarrelTrans = SMI_Transform();

			BarrelTrans.setPos(glm::vec3(-0.2, -6, 1));

			BarrelTrans.SetDegree(glm::vec3(90, -10, 90));
			AttachCopy(barrel, BarrelTrans);
		}
		VertexArrayObject::Sptr win = ObjLoader::LoadFromFile("Models/window1.obj");
		{
			wa1 = CreateEntity();
			//create texture
			Texture2D::Sptr WinTexture = Texture2D::Create("Textures/BrownTex.1001.png");
			//material
			SMI_Material::Sptr BarrelMa = SMI_Material::Create();


			BarrelMa->setShader(shader);

			//set textures
			BarrelMa->setTexture(WinTexture, 0);
			//render
			Renderer BarrelRen = Renderer(BarrelMa, win);
			AttachCopy(wa1, BarrelRen);
			//transform
			SMI_Transform winTrans1 = SMI_Transform();

			winTrans1.setPos(glm::vec3(-10.2, -6, 1));

			winTrans1.SetDegree(glm::vec3(90, -10, 90));
			AttachCopy(wa1, winTrans1);
		}
		VertexArrayObject::Sptr win1 = ObjLoader::LoadFromFile("Models/window1.obj");
		{
			wa2 = CreateEntity();
			Texture2D::Sptr window2Texture = Texture2D::Create("Textures/BrownTex.1001.png");
			//material
			SMI_Material::Sptr BarrelM = SMI_Material::Create();
			BarrelM->setShader(shader);

			//set textures
			BarrelM->setTexture(window2Texture, 0);
			//render
			Renderer BarrelRe = Renderer(BarrelM, win1);
			AttachCopy(wa2, BarrelRe);
			//transform
			SMI_Transform winTrans = SMI_Transform();

			winTrans.setPos(glm::vec3(-20.2, -6, 1));

			winTrans.SetDegree(glm::vec3(90, -10, 90));
			AttachCopy(wa2, winTrans);
		}
		VertexArrayObject::Sptr win2 = ObjLoader::LoadFromFile("Models/window1.obj");
		{
			wa3 = CreateEntity();

			//create texture
			Texture2D::Sptr Window3Texture = Texture2D::Create("Textures/BrownTex.1001.png");
			//material
			SMI_Material::Sptr windowmat = SMI_Material::Create();
			windowmat->setShader(shader);

			//set textures
			windowmat->setTexture(Window3Texture, 0);
			//render
			Renderer windowren = Renderer(windowmat, win2);
			AttachCopy(wa3, windowren);
			//transform
			SMI_Transform windowTrans = SMI_Transform();

			windowTrans.setPos(glm::vec3(-39.8, -6, -3.7));

			windowTrans.SetDegree(glm::vec3(90, -10, 90));
			AttachCopy(wa3, windowTrans);
		}
		VertexArrayObject::Sptr win3 = ObjLoader::LoadFromFile("Models/window1.obj");
		{
			wa3 = CreateEntity();

			//create texture
			Texture2D::Sptr win3Texture = Texture2D::Create("Textures/BrownTex.1001.png");
			//material
			SMI_Material::Sptr windowmat1 = SMI_Material::Create();
			windowmat1->setShader(shader);

			//set textures
			windowmat1->setTexture(win3Texture, 0);
			//render
			Renderer windowren1 = Renderer(windowmat1, win3);
			AttachCopy(wa3, windowren1);
			//transform
			SMI_Transform windowTrans1 = SMI_Transform();

			windowTrans1.setPos(glm::vec3(-49.8, -6, -3.7));

			windowTrans1.SetDegree(glm::vec3(90, -10, 90));
			AttachCopy(wa3, windowTrans1);
		}
		VertexArrayObject::Sptr win4 = ObjLoader::LoadFromFile("Models/window1.obj");
		{
			wa4 = CreateEntity();

			//create texture
			Texture2D::Sptr win5Texture = Texture2D::Create("Textures/BrownTex.1001.png");
			//material
			SMI_Material::Sptr windowmat2 = SMI_Material::Create();
			windowmat2->setShader(shader);

			//set textures
			windowmat2->setTexture(win5Texture, 0);
			//render
			Renderer windowren2 = Renderer(windowmat2, win4);
			AttachCopy(wa4, windowren2);
			//transform
			SMI_Transform windowTrans2 = SMI_Transform();

			windowTrans2.setPos(glm::vec3(-60.8, -6, -3.7));

			windowTrans2.SetDegree(glm::vec3(90, -10, 90));
			AttachCopy(wa4, windowTrans2);
		}
		VertexArrayObject::Sptr vao5 = ObjLoader::LoadFromFile("Models/barrel1.obj");
		{

			barrel1 = CreateEntity();

			//create texture
			Texture2D::Sptr BarrelTexture = Texture2D::Create("Textures/Barrel.png");

			//material
			SMI_Material::Sptr BarrelMat1 = SMI_Material::Create();
			BarrelMat1->setShader(shader);

			//set textures
			BarrelMat1->setTexture(BarrelTexture, 0);

			//render
			Renderer BarrelRend1 = Renderer(BarrelMat1, vao5);
			AttachCopy(barrel1, BarrelRend1);
			//transform
			SMI_Transform BarrelTrans1 = SMI_Transform();

			BarrelTrans1.setPos(glm::vec3(-0.6, 0, 2.7));

			BarrelTrans1.SetDegree(glm::vec3(0, 90, 0));
			AttachCopy(barrel1, BarrelTrans1);
		}


		VertexArrayObject::Sptr vao6 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			barrel2 = CreateEntity();

			Texture2D::Sptr floortex = Texture2D::Create("Textures/Untitled.1001.png");
			//material
			SMI_Material::Sptr BarrelMat2 = SMI_Material::Create();
			BarrelMat2->setShader(shader);

			//set textures
			BarrelMat2->setTexture(floortex, 0);
			//render
			Renderer BarrelRend2 = Renderer(BarrelMat2, vao6);
			AttachCopy(barrel2, BarrelRend2);
			//transform
			SMI_Transform BarrelTrans2 = SMI_Transform();

			BarrelTrans2.setPos(glm::vec3(0.03, 0, -0.8));

			BarrelTrans2.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(barrel2, BarrelTrans2);
		}

		VertexArrayObject::Sptr vao7 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			barrel3 = CreateEntity();

			Texture2D::Sptr floor1Texture = Texture2D::Create("Textures/Untitled.1001.png");
			//material
			SMI_Material::Sptr BarrelMat3 = SMI_Material::Create();
			BarrelMat3->setShader(shader);

			//set textures
			BarrelMat3->setTexture(floor1Texture, 0);
			//render
			Renderer BarrelRend3 = Renderer(BarrelMat3, vao7);
			AttachCopy(barrel3, BarrelRend3);
			//transform
			SMI_Transform BarrelTrans3 = SMI_Transform();

			BarrelTrans3.setPos(glm::vec3(-1.8, 0, -0.8));

			BarrelTrans3.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(barrel3, BarrelTrans3);
		}
		VertexArrayObject::Sptr vao8 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			barrel4 = CreateEntity();

			Texture2D::Sptr window10Texture = Texture2D::Create("Textures/Untitled.1001.png");
			//material
			SMI_Material::Sptr BarrelMat4 = SMI_Material::Create();
			BarrelMat4->setShader(shader);

			//set textures
			BarrelMat4->setTexture(window10Texture, 0);
			//render
			Renderer BarrelRend4 = Renderer(BarrelMat4, vao8);
			AttachCopy(barrel4, BarrelRend4);
			//transform
			SMI_Transform BarrelTrans4 = SMI_Transform();

			BarrelTrans4.setPos(glm::vec3(-12.8, 0, -0.8));

			BarrelTrans4.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(barrel4, BarrelTrans4);
		}
		VertexArrayObject::Sptr wall3 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			walls3 = CreateEntity();

			Texture2D::Sptr wall3Texture = Texture2D::Create("Textures/Untitled.1001.png");
			//material
			SMI_Material::Sptr WallMat = SMI_Material::Create();
			WallMat->setShader(shader);

			//set textures
			WallMat->setTexture(wall3Texture, 0);
			//render
			Renderer WallRend = Renderer(WallMat, wall3);
			AttachCopy(walls3, WallRend);
			//transform
			SMI_Transform WallTrans = SMI_Transform();

			WallTrans.setPos(glm::vec3(-23.8, 0, -0.8));

			WallTrans.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(walls3, WallTrans);
		}
		VertexArrayObject::Sptr wall4 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			walls4 = CreateEntity();

			Texture2D::Sptr window90Texture = Texture2D::Create("Textures/Untitled.1001.png");
			//material
			SMI_Material::Sptr WallMat1 = SMI_Material::Create();
			WallMat1->setShader(shader);

			//set textures
			WallMat1->setTexture(window90Texture, 0);
			//render
			Renderer WallRend1 = Renderer(WallMat1, wall4);
			AttachCopy(walls4, WallRend1);
			//transform
			SMI_Transform WallTrans1 = SMI_Transform();

			WallTrans1.setPos(glm::vec3(-30.8, 0, -0.8));

			WallTrans1.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(walls4, WallTrans1);
		}

		VertexArrayObject::Sptr crate = ObjLoader::LoadFromFile("Models/Crates1.obj");
		{

			crate1 = CreateEntity();

			Texture2D::Sptr crateTex = Texture2D::Create("Textures/box3.png");
			//material
			SMI_Material::Sptr BarrelMat5 = SMI_Material::Create();
			BarrelMat5->setShader(shader);

			//set textures
			BarrelMat5->setTexture(crateTex, 0);
			//render
			Renderer BarrelRend5 = Renderer(BarrelMat5, crate);
			AttachCopy(crate1, BarrelRend5);
			//transform
			SMI_Transform BarrelTrans5 = SMI_Transform();

			BarrelTrans5.setPos(glm::vec3(-16.8, -2, 3.8));

			BarrelTrans5.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(crate1, BarrelTrans5);
		}
		VertexArrayObject::Sptr door = ObjLoader::LoadFromFile("Models/Door2.obj");
		{

			door1 = CreateEntity();

			Texture2D::Sptr doorTexture = Texture2D::Create("Textures/DoorOpen.png");
			//material
			SMI_Material::Sptr BarrelMat6 = SMI_Material::Create();
			BarrelMat6->setShader(shader);

			//set textures
			BarrelMat6->setTexture(doorTexture, 0);
			//render
			Renderer BarrelRend6 = Renderer(BarrelMat6, door);
			AttachCopy(door1, BarrelRend6);
			//transform
			SMI_Transform BarrelTrans6 = SMI_Transform();

			BarrelTrans6.setPos(glm::vec3(-27.2, 0, 2));

			BarrelTrans6.SetDegree(glm::vec3(90, 0, -180));
			AttachCopy(door1, BarrelTrans6);
		}

		VertexArrayObject::Sptr crate1 = ObjLoader::LoadFromFile("Models/Crates1.obj");
		{

			crate2 = CreateEntity();

			Texture2D::Sptr crate1Texture = Texture2D::Create("Textures/box3.png");
			//material
			SMI_Material::Sptr BarrelMat5 = SMI_Material::Create();
			BarrelMat5->setShader(shader);

			//set textures
			BarrelMat5->setTexture(crate1Texture, 0);
			//render
			Renderer BarrelRend5 = Renderer(BarrelMat5, crate1);
			AttachCopy(crate2, BarrelRend5);
			//transform
			SMI_Transform BarrelTrans5 = SMI_Transform();

			BarrelTrans5.setPos(glm::vec3(-16.8, -2, 2.2));

			BarrelTrans5.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(crate2, BarrelTrans5);
		}
		VertexArrayObject::Sptr w = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			w1 = CreateEntity();

			Texture2D::Sptr window1Texture6 = Texture2D::Create("Textures/BrownTex.1001.png");
			//material
			SMI_Material::Sptr WallMat6 = SMI_Material::Create();
			WallMat6->setShader(shader);

			//set textures
			WallMat6->setTexture(window1Texture6, 0);
			//render
			Renderer WallRend6 = Renderer(WallMat6, w);
			AttachCopy(w1, WallRend6);
			//transform
			SMI_Transform WallTrans6 = SMI_Transform();

			WallTrans6.setPos(glm::vec3(-45.8, 0, -5.6));

			WallTrans6.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(w1, WallTrans6);
		}
		VertexArrayObject::Sptr shelf = ObjLoader::LoadFromFile("Models/shelf12.obj");
		{

			shelf1 = CreateEntity();

			Texture2D::Sptr ShelfTex = Texture2D::Create("Textures/shelf.png");
			//material
			SMI_Material::Sptr WallMat7 = SMI_Material::Create();
			WallMat7->setShader(shader);

			//set textures
			WallMat7->setTexture(ShelfTex, 0);
			//render
			Renderer WallRend7 = Renderer(WallMat7, shelf);
			AttachCopy(shelf1, WallRend7);
			//transform
			SMI_Transform WallTrans7 = SMI_Transform();

			WallTrans7.setPos(glm::vec3(-49.8, -1.2, -0.6));

			WallTrans7.SetDegree(glm::vec3(0, 0, 0));
			AttachCopy(shelf1, WallTrans7);
		}
		VertexArrayObject::Sptr fan = ObjLoader::LoadFromFile("Models/cfan1.obj");
		{

			fan1 = CreateEntity();

			Texture2D::Sptr cfanTexture = Texture2D::Create("Textures/fan.png");
			//material
			SMI_Material::Sptr fanmat = SMI_Material::Create();
			fanmat->setShader(shader);

			//set textures
			fanmat->setTexture(cfanTexture, 0);
			//render
			Renderer fanrend = Renderer(fanmat, fan);
			AttachCopy(fan1, fanrend);
			//transform
			SMI_Transform fanTrans = SMI_Transform();

			fanTrans.setPos(glm::vec3(-50.8, -2, 1.8));

			fanTrans.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(fan1, fanTrans);
		}
		VertexArrayObject::Sptr fanHolder = ObjLoader::LoadFromFile("Models/cholder.obj");
		{

			fanH = CreateEntity();

			Texture2D::Sptr fanholderTexture = Texture2D::Create("Textures/Barrel.png");
			//material
			SMI_Material::Sptr fanmat1 = SMI_Material::Create();
			fanmat1->setShader(shader);

			//set textures
			fanmat1->setTexture(fanholderTexture, 0);
			//render
			Renderer fanrend1 = Renderer(fanmat1, fanHolder);
			AttachCopy(fanH, fanrend1);
			//transform
			SMI_Transform fanTrans1 = SMI_Transform();

			fanTrans1.setPos(glm::vec3(-50.8, -2, 1.8));

			fanTrans1.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(fanH, fanTrans1);
		}
		VertexArrayObject::Sptr wall5 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			walls5 = CreateEntity();

			Texture2D::Sptr wall5Texture = Texture2D::Create("Textures/BrownTex.1001.png");
			//material
			SMI_Material::Sptr WallMat10 = SMI_Material::Create();
			WallMat10->setShader(shader);

			//set textures
			WallMat10->setTexture(wall5Texture, 0);
			//render
			Renderer WallRend10 = Renderer(WallMat10, wall5);
			AttachCopy(walls5, WallRend10);
			//transform
			SMI_Transform WallTrans10 = SMI_Transform();

			WallTrans10.setPos(glm::vec3(-57.2, 0, -5.6));

			WallTrans10.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(walls5, WallTrans10);
		}
		VertexArrayObject::Sptr shelf1 = ObjLoader::LoadFromFile("Models/shelf12.obj");
		{

			shel = CreateEntity();

			Texture2D::Sptr shelf10Texture = Texture2D::Create("Textures/shelf.png");
			//material
			SMI_Material::Sptr ShelfM = SMI_Material::Create();
			ShelfM->setShader(shader);

			//set textures
			ShelfM->setTexture(shelf10Texture, 0);
			//render
			Renderer ShelfRen7 = Renderer(ShelfM, shelf1);
			AttachCopy(shel, ShelfRen7);
			//transform
			SMI_Transform ShelfTrans7 = SMI_Transform();

			ShelfTrans7.setPos(glm::vec3(-59.8, 0, -2.6));

			ShelfTrans7.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(shel, ShelfTrans7);
		}


		VertexArrayObject::Sptr w3 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			f = CreateEntity();

			Texture2D::Sptr w3tex = Texture2D::Create("Textures/road.png");
			//material
			SMI_Material::Sptr floormat2 = SMI_Material::Create();
			floormat2->setShader(shader);

			//set textures
			floormat2->setTexture(w3tex, 0);
			//render
			Renderer WallRend6 = Renderer(floormat2, w3);
			AttachCopy(f, WallRend6);
			//transform
			SMI_Transform floorTrans1 = SMI_Transform();

			floorTrans1.setPos(glm::vec3(-79.8, 0, 0.6));

			floorTrans1.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(f, floorTrans1);
		}
		VertexArrayObject::Sptr w4 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			f1 = CreateEntity();

			Texture2D::Sptr w4tex = Texture2D::Create("Textures/road.png");
			//material
			SMI_Material::Sptr floormat3 = SMI_Material::Create();
			floormat3->setShader(shader);

			//set textures
			floormat3->setTexture(w4tex, 0);
			//render
			Renderer WallRend7 = Renderer(floormat3, w4);
			AttachCopy(f1, WallRend7);
			//transform
			SMI_Transform floorTrans2 = SMI_Transform();

			floorTrans2.setPos(glm::vec3(-88.8, 0, 0.6));

			floorTrans2.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(f1, floorTrans2);
		}
		VertexArrayObject::Sptr w5 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			f2 = CreateEntity();

			Texture2D::Sptr w5Texture = Texture2D::Create("Textures/road.png");
			//material
			SMI_Material::Sptr floormat4 = SMI_Material::Create();
			floormat4->setShader(shader);

			//set textures
			floormat4->setTexture(w5Texture, 0);
			//render
			Renderer WallRend13 = Renderer(floormat4, w5);
			AttachCopy(f2, WallRend13);
			//transform
			SMI_Transform floorTrans4 = SMI_Transform();

			floorTrans4.setPos(glm::vec3(-99.8, 0, 0.6));

			floorTrans4.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(f2, floorTrans4);
		}
		VertexArrayObject::Sptr w6 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			f3 = CreateEntity();

			Texture2D::Sptr w6Texture = Texture2D::Create("Textures/Road.png");
			//material
			SMI_Material::Sptr floormat5 = SMI_Material::Create();
			floormat5->setShader(shader);

			//set textures
			floormat5->setTexture(w6Texture, 0);
			//render
			Renderer WallRend15 = Renderer(floormat5, w6);
			AttachCopy(f3, WallRend15);
			//transform
			SMI_Transform floorTrans5 = SMI_Transform();

			floorTrans5.setPos(glm::vec3(-119.8, 0, 0.6));

			floorTrans5.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(f3, floorTrans5);
		}
		VertexArrayObject::Sptr w7 = ObjLoader::LoadFromFile("Models/nba1.obj");
		{

			L_plat = CreateEntity();

			Texture2D::Sptr w7Texture = Texture2D::Create("Textures/Road.png");
			//material
			SMI_Material::Sptr floormat90 = SMI_Material::Create();
			floormat90->setShader(shader);

			//set textures
			floormat90->setTexture(w7Texture, 0);
			//render
			Renderer WallRend195 = Renderer(floormat90, w7);
			AttachCopy(L_plat, WallRend195);
			//transform
			SMI_Transform floorTrans905 = SMI_Transform();

			floorTrans905.setPos(glm::vec3(-129.8, 0, 0.6));

			floorTrans905.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(L_plat, floorTrans905);
		}

		VertexArrayObject::Sptr cars = ObjLoader::LoadFromFile("Models/car.obj");
		{

			car = CreateEntity();

			Texture2D::Sptr carsTexture = Texture2D::Create("Textures/car_Tex.png");
			//material
			SMI_Material::Sptr carM = SMI_Material::Create();
			carM->setShader(shader);

			//set textures
			carM->setTexture(carsTexture, 0);
			//render
			Renderer carRen7 = Renderer(carM, cars);
			AttachCopy(car, carRen7);
			//transform
			SMI_Transform carTrans = SMI_Transform();

			carTrans.setPos(glm::vec3(-88.8, 0, 3.4));

			carTrans.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(car, carTrans);
		}
		VertexArrayObject::Sptr building1 = ObjLoader::LoadFromFile("Models/building1.obj");
		{
			build = CreateEntity();

			Texture2D::Sptr build1Texture = Texture2D::Create("Textures/build.png");
			//material
			SMI_Material::Sptr buildM = SMI_Material::Create();
			buildM->setShader(shader);

			//set textures
			buildM->setTexture(build1Texture, 0);
			//render
			Renderer buildRen7 = Renderer(buildM, building1);
			AttachCopy(build, buildRen7);
			//transform
			SMI_Transform buildTrans = SMI_Transform();

			buildTrans.setPos(glm::vec3(-79.8, -6, 3.2));

			buildTrans.SetDegree(glm::vec3(90, 0, -90));
			AttachCopy(build, buildTrans);
		}

		VertexArrayObject::Sptr building2 = ObjLoader::LoadFromFile("Models/building1.obj");
		{

			build2 = CreateEntity();

			Texture2D::Sptr build2Texture = Texture2D::Create("Textures/build.png");
			//material
			SMI_Material::Sptr buildM1 = SMI_Material::Create();
			buildM1->setShader(shader);

			//set textures
			buildM1->setTexture(build2Texture, 0);
			//render
			Renderer build1Ren7 = Renderer(buildM1, building2);
			AttachCopy(build2, build1Ren7);
			//transform
			SMI_Transform buildTrans1 = SMI_Transform();

			buildTrans1.setPos(glm::vec3(-94.8, -6, 3.2));

			buildTrans1.SetDegree(glm::vec3(90, 0, -90));
			AttachCopy(build2, buildTrans1);
		}
		VertexArrayObject::Sptr building3 = ObjLoader::LoadFromFile("Models/build2.obj");
		{

			build3 = CreateEntity();

			Texture2D::Sptr build3Texture = Texture2D::Create("Textures/build2.png");
			//material
			SMI_Material::Sptr buildM12 = SMI_Material::Create();
			buildM12->setShader(shader);

			//set textures
			buildM12->setTexture(build3Texture, 0);
			//render
			Renderer build1Ren71 = Renderer(buildM12, building3);
			AttachCopy(build3, build1Ren71);
			//transform
			SMI_Transform buildTrans12 = SMI_Transform();

			buildTrans12.setPos(glm::vec3(-87.8, -6, 3.2));

			buildTrans12.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(build3, buildTrans12);
		}
		VertexArrayObject::Sptr elevator1 = ObjLoader::LoadFromFile("Models/elevator.obj");
		{

			elevator12 = CreateEntity();

			Texture2D::Sptr elevatorTexture = Texture2D::Create("Textures/DoorO.png");
			//material
			SMI_Material::Sptr WallMat4 = SMI_Material::Create();
			WallMat4->setShader(shader);

			//set textures
			WallMat4->setTexture(elevatorTexture, 0);
			//render
			Renderer WallRend12 = Renderer(WallMat4, elevator1);
			AttachCopy(elevator12, WallRend12);
			//transform
			SMI_Transform WallTrans3 = SMI_Transform();

			WallTrans3.setPos(glm::vec3(-68.8, -1.6, -0.6));

			WallTrans3.SetDegree(glm::vec3(90, 0, 0));
			AttachCopy(elevator12, WallTrans3);
		}
		VertexArrayObject::Sptr Table1 = ObjLoader::LoadFromFile("Models/project.obj");
		{

			table = CreateEntity();

			Texture2D::Sptr elevatorTexture = Texture2D::Create("Textures/Table_Mat.png");
			//material
			SMI_Material::Sptr WallMat41 = SMI_Material::Create();
			WallMat41->setShader(shader);

			//set textures
			WallMat41->setTexture(elevatorTexture, 0);
			//render
			Renderer WallRend121 = Renderer(WallMat41, Table1);
			AttachCopy(table, WallRend121);
			//transform
			SMI_Transform WallTrans31 = SMI_Transform();

			WallTrans31.setPos(glm::vec3(-3.6, 0, 3.4));

			WallTrans31.SetDegree(glm::vec3(90, 0, 0));
			AttachCopy(table, WallTrans31);
		}
		VertexArrayObject::Sptr plank1 = ObjLoader::LoadFromFile("Models/plank.obj");
		{

			plank = CreateEntity();

			Texture2D::Sptr plankTexture = Texture2D::Create("Textures/platform.png");
			//material
			SMI_Material::Sptr WallMat42 = SMI_Material::Create();
			WallMat42->setShader(shader);

			//set textures
			WallMat42->setTexture(plankTexture, 0);
			//render
			Renderer WallRend120 = Renderer(WallMat42, plank1);
			AttachCopy(plank, WallRend120);
			//transform
			SMI_Transform WallTrans35 = SMI_Transform();

			WallTrans35.setPos(glm::vec3(-103.8, -1.6, 5.6));

			WallTrans35.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(plank, WallTrans35);
		}
		VertexArrayObject::Sptr Roadblock = ObjLoader::LoadFromFile("Models/road block.obj");
		{

			roadb = CreateEntity();

			Texture2D::Sptr roadbTexture = Texture2D::Create("Textures/road_bock.png");
			//material
			SMI_Material::Sptr lMat42 = SMI_Material::Create();
			lMat42->setShader(shader);

			//set textures
			lMat42->setTexture(roadbTexture, 0);
			//render
			Renderer WallRend120 = Renderer(lMat42, Roadblock);
			AttachCopy(roadb, WallRend120);
			//transform
			SMI_Transform WallTrans356 = SMI_Transform();

			WallTrans356.setPos(glm::vec3(-105.8, 1.2, 3.3));

			WallTrans356.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(roadb, WallTrans356);
		}
		VertexArrayObject::Sptr Roadblock1 = ObjLoader::LoadFromFile("Models/road block.obj");
		{

			roadb1 = CreateEntity();

			Texture2D::Sptr roadbTexture1 = Texture2D::Create("Textures/road_bock.png");
			//material
			SMI_Material::Sptr lMat421 = SMI_Material::Create();
			lMat421->setShader(shader);

			//set textures
			lMat421->setTexture(roadbTexture1, 0);
			//render
			Renderer WallRend1201 = Renderer(lMat421, Roadblock1);
			AttachCopy(roadb1, WallRend1201);
			//transform
			SMI_Transform WallTrans3561 = SMI_Transform();

			WallTrans3561.setPos(glm::vec3(-105.8, -1.2, 3.3));

			WallTrans3561.SetDegree(glm::vec3(90, 0, 90));
			AttachCopy(roadb1, WallTrans3561);
		}
		VertexArrayObject::Sptr bar = ObjLoader::LoadFromFile("Models/bartable.obj");
		{
			bartab = CreateEntity();
			//create texture
			Texture2D::Sptr WinTexture80 = Texture2D::Create("Textures/BrownTex.1001.png");
			//material
			SMI_Material::Sptr BarrelMa80 = SMI_Material::Create();


			BarrelMa80->setShader(shader);

			//set textures
			BarrelMa80->setTexture(WinTexture80, 0);
			//render
			Renderer BarrelRen80 = Renderer(BarrelMa80, bar);
			AttachCopy(bartab, BarrelRen80);
			//transform
			SMI_Transform winTrans180 = SMI_Transform();

			winTrans180.setPos(glm::vec3(-0.2, 0, 2.7));

			winTrans180.SetDegree(glm::vec3(90, 0, -90));
			AttachCopy(bartab, winTrans180);
		}

		VertexArrayObject::Sptr garbage1 = ObjLoader::LoadFromFile("Models/garbage bin.obj");
		{
			garbage = CreateEntity();
			//create texture
			Texture2D::Sptr gTexture80 = Texture2D::Create("Textures/bin.png");
			//material
			SMI_Material::Sptr gMa80 = SMI_Material::Create();


			gMa80->setShader(shader);

			//set textures
			gMa80->setTexture(gTexture80, 0);
			//render
			Renderer gRen80 = Renderer(gMa80, garbage1);
			AttachCopy(garbage, gRen80);
			//transform
			SMI_Transform winTrans1801 = SMI_Transform();

			winTrans1801.setPos(glm::vec3(-8.7, 0, 2.7));

			winTrans1801.SetDegree(glm::vec3(90, 0, -90));
			AttachCopy(garbage, winTrans1801);
		}


	}

	void Update(float deltaTime)
	{
		//increment time
		current += deltaTime;
		c += deltaTime;
		if (current > max)
		{
			current = max;
		}
		if (c > max)
		{
			c = 0;

		}
		float t = current / max;
		float time = c / max;

		//rotation example
		GetComponent<SMI_Transform>(fan1).FixedRotate(glm::vec3(0, 0, 30) * deltaTime * 8.0f);

		// LERP example

		GetComponent<SMI_Transform>(door1).setPos(Lerp(glm::vec3(-27.2, 0, 2), glm::vec3(-27.2, 0, 10), t));



		GetComponent<SMI_Transform>(car).setPos(Lerp(glm::vec3(-99.8, 3, 3.8), glm::vec3(-74.8, 3, 3.8), time));



		GetComponent<SMI_Transform>(elevator12).setPos(Lerp(glm::vec3(-68.8, -1.6, -7.6), glm::vec3(-68.8, -1.6, 0.6), time));

		GetComponent<SMI_Transform>(f3).setPos(Lerp(glm::vec3(-100.8, 0, -4.6), glm::vec3(-119.8, 0, 0.6), time));

		SMI_Scene::Update(deltaTime);
	}

	~GameScene() = default;

private:
	entt::entity barrel;
	entt::entity barrel1;
	entt::entity barrel2;
	entt::entity barrel3;
	entt::entity barrel4;
	entt::entity crate1;
	entt::entity door1;
	entt::entity crate2;
	entt::entity walls3;
	entt::entity walls4;
	entt::entity elevator1;
	entt::entity elevator12;
	entt::entity w1;
	entt::entity shelf1;
	entt::entity fan1;
	entt::entity walls5;
	entt::entity shel;
	entt::entity fanH;
	entt::entity f;
	entt::entity f1;
	entt::entity f2;
	entt::entity f3;
	entt::entity car;
	entt::entity build;
	entt::entity build2;
	entt::entity build3;
	entt::entity road1;
	entt::entity wa1;
	entt::entity wa2;
	entt::entity wa3;
	entt::entity wa4;
	entt::entity bartab;
	entt::entity chair;
	entt::entity table;
	entt::entity L_plat;
	entt::entity plank;
	entt::entity roadb;
	entt::entity roadb1;
	entt::entity garbage;

	float max = 5;
	float current = 0;
	float c = 0;
};

//main game loop inside here as well as call all needed shaders
int main()
{
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlDebugMessage, nullptr);

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	GameScene MainScene = GameScene();
	MainScene.InitScene();

	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();


		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MainScene.Update(dt);

		MainScene.Render();

		lastFrame = thisFrame;
		glfwSwapBuffers(window);
	}

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}