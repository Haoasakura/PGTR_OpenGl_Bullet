#include <iostream>
#include "GameManager.h"
#include "Camera.h";
#include <btBulletDynamicsCommon.h>

bool firstMouse = true;
float lastX = 640, lastY = 360;

GameManager *gameManager;
Camera *camera;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main() {

	camera = &Camera::getCamera();
	gameManager = &GameManager::getGameManager();

	glfwSetFramebufferSizeCallback(gameManager->getWindow(), framebuffer_size_callback);
	glfwSetCursorPosCallback(gameManager->getWindow(), mouse_callback);
	glfwSetScrollCallback(gameManager->getWindow(), scroll_callback);
	
	gameManager->runGameLoop();
	
	GameManager::destroyGameManager();
	return 0;
}


void framebuffer_size_callback(GLFWwindow * window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera->ProcessMouseScroll(yoffset);
}
