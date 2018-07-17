#include "GameManager.h"

GameManager* GameManager::gameManager = nullptr;

GameManager::GameManager(): _window(glfwGetCurrentContext()), _renderSystem(&RenderSystem::getRenderSystem())
{
	_camera = &Camera::getCamera();

	deltaTime = 0.0f;
	lastFrame = 0.0f;
}


GameManager::~GameManager()
{
	glfwTerminate();
	RenderSystem::destroyRenderSystem();
}

GameManager& GameManager::getGameManager() {
		
	if (gameManager == nullptr) {
		
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);
		//GLFWmonitor* primary = glfwGetPrimaryMonitor();
		GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PGTR", NULL, NULL);
		glfwMakeContextCurrent(window);
		
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
		}

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		gameManager = new GameManager();
		glfwSetInputMode(gameManager->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	}
	return *gameManager;
}

void GameManager::destroyGameManager() {

	GameManager *gameManager = &getGameManager();
	
	delete gameManager;
	GLFWwindow *window = glfwGetCurrentContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void GameManager::runGameLoop() {

	while (!glfwWindowShouldClose(_window)) {

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(_window);

		_renderSystem->render();
	}
}

GLFWwindow* GameManager::getWindow()
{
	return _window;
}

void GameManager::processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		_camera->ProcessKeyboard(FORWARD, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		_camera->ProcessKeyboard(BACKWARD, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		_camera->ProcessKeyboard(LEFT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		_camera->ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		_camera->ProcessKeyboard(UPWARD, deltaTime);
}
