#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "RenderSystem.h"
#include "stb_image.h"
#include "Camera.h"

using namespace std;

class GameManager
{

public:

	static GameManager& getGameManager();
	static void destroyGameManager();

	void runGameLoop();
	GLFWwindow* getWindow();

private:

	static const unsigned int SCR_WIDTH = 1280;
	static const unsigned int SCR_HEIGHT = 720;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	static GameManager *gameManager;

	GLFWwindow *_window;
	Camera *_camera;
	RenderSystem *_renderSystem;
	
	GameManager();
	~GameManager();

	void processInput(GLFWwindow *window);

};

