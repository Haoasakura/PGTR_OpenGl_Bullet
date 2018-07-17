#pragma once

#include <glad\glad.h>
#define GLFW_INCLUDE_GLU
#include <GLFW\glfw3.h>
#include <iostream>
#include <vector>
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>

#include "BulletWorld.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture2D.h"

using namespace std;

class BulletWorld;
class Camera;

class RenderSystem
{

public:

	static const unsigned int SCR_WIDTH = 1280;
	static const unsigned int SCR_HEIGHT = 720;

	static RenderSystem& getRenderSystem();
	static void destroyRenderSystem();

	void render();
	
private:

	bool bloom;
	float exposure;

	unsigned int dustVBO, bubblesVBO, quadVAO, quadVBO, framebuffer, textureColorbuffer;
	unsigned int dustAmount, bubblesAmount, ballsAmount, boxesAmount;
	unsigned int hdrFBO, rboDepth;
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	unsigned int pingpongFBOs[2];
	unsigned int pingpongColorbuffers[2];
	unsigned int colorBuffers[2];

	Model *cubeModel, *sphereModel, *dustModel;
	glm::mat4 *dustModelMatrices, *bubblesModelMatrices;

	static RenderSystem *renderSystem;
	static map<string, Shader*> Shaders;
	static map<string, Texture2D*> Textures;

	BulletWorld *_world;
	GLFWwindow *_window;
	Camera *_camera;

	RenderSystem();
	~RenderSystem();

	void initializeShaders();
	void initializeTextures();
	void initializeModels();
	void initializeScreenQuad();
	void initializeDust();
	void initializeBubbles();

	void addShader(Shader *shader, string name);
	void addTexture(Texture2D *texture, string name);
	void addRooms(string name);
	void addBallsToBounce(string name, unsigned int amount);
	void addBoxesToShake(string name, unsigned int amount);

	Shader* RenderSystem::getShader(string name);
	Texture2D* RenderSystem::getTexture(string name);

	void setupLightsParameter(glm::vec3 dirAmbient, glm::vec3 pointPosition);
	
	void renderRoom1(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color);
	void renderRoom2(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color);
	void renderRoom3(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color);
	void renderRoom4(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color);
	void renderRoom5(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color);
	void renderRoom6(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color);
	void renderBox(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color);
	void renderSphere(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color);
	void renderBallsToBounce(string name, glm::mat4 projection, glm::mat4 view, unsigned int amount);
	void renderBoxesToShake(string name, glm::mat4 projection, glm::mat4 view, unsigned int amount);
	void renderDust(glm::mat4 projection, glm::mat4 view, unsigned int amount, glm::mat4* modelMatrices);
	void renderWaterWaves(glm::mat4 projection, glm::mat4 view);
	void renderBubbles(glm::mat4 projection, glm::mat4 view, unsigned int amount, glm::mat4* modelMatrices);
	void renderScreen();
	
	void applyBloom();
	void applyWind();
	void applyEathquake();
	void applyUnderwater();

	//glm::mat4 RenderSystem::bulletToGlm(const btTransform& t);
};

