#include "RenderSystem.h"

float quadVertices[] = {
	// positions           // texture Coords
	-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,		1.0f, 1.0f,
	 1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
};

RenderSystem* RenderSystem::renderSystem = nullptr;
GLUquadricObj *quad;
BulletWorld *_world = &BulletWorld::getBulletWorld();
map<string, Shader*> RenderSystem::Shaders;
map<string, Texture2D*> RenderSystem::Textures;

RenderSystem::RenderSystem(): _window(glfwGetCurrentContext()) {

	_world = &BulletWorld::getBulletWorld();
	_camera = &Camera::getCamera();
	quad = gluNewQuadric();	

	dustAmount = 20000;
	bubblesAmount = 5000;
	ballsAmount = 25;
	boxesAmount = 10;
	exposure = 1.0f;
	bloom = false;

	initializeShaders();
	initializeTextures();
	initializeScreenQuad();
	initializeModels();
	
	_world->addFloor("floor", glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 0.0f);

	addRooms("room");

	_world->addBox("underwaterBox", 1.0f, 1.0f, 1.0f, -20.0f, 17.0f, 10.0f, 1.0f);
	_world->getBody("underwaterBox")->setLinearVelocity(btVector3(0, 0, 0));
	_world->getBody("underwaterBox")->setAngularFactor(btVector3(1, 1, 1));
	_world->getBody("underwaterBox")->forceActivationState(true);

	addBoxesToShake("cont", boxesAmount);
	addBallsToBounce("ball", ballsAmount);
	
	initializeDust();
	initializeBubbles();
}

RenderSystem::~RenderSystem()
{
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &dustVBO);
	glDeleteBuffers(1, &bubblesVBO);
	glDeleteBuffers(1, &quadVAO);
	glDeleteBuffers(1, &framebuffer);
	glDeleteBuffers(1, &textureColorbuffer);
	glDeleteBuffers(1, &hdrFBO);
	glDeleteBuffers(1, &rboDepth);
	glDeleteBuffers(2, attachments);
	glDeleteBuffers(2, pingpongFBOs);
	glDeleteBuffers(2, pingpongColorbuffers);
	glDeleteBuffers(2, colorBuffers);

	for (auto iter : Shaders)
		glDeleteProgram(iter.second->programID);

	gluDeleteQuadric(quad);

	delete cubeModel;
	delete sphereModel;
	delete dustModel;
	delete _world;
}

RenderSystem& RenderSystem::getRenderSystem() {
	
	if (renderSystem == nullptr) {
		renderSystem = new RenderSystem();
	}

	return *renderSystem;
}

void RenderSystem::destroyRenderSystem() {
	RenderSystem *renderSystem = &getRenderSystem();
	delete renderSystem;
}

void RenderSystem::render() {
	
	applyWind();
	applyEathquake();
	applyUnderwater();
	_world->stepSimulate();

	btVector3 cam = _world->getBody("player")->getCenterOfMassPosition();
	_camera->Position = glm::vec3(cam.getX(), cam.getY(), cam.getZ());

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(_camera->Zoom), 1280.0f / 720.0f, 0.1f, 100.0f);
	glm::mat4 view = _camera->GetViewMatrix();

	setupLightsParameter(glm::vec3(0.60f, 0.65f, 1.0f), glm::vec3(-20.0f, 10.0f, 10.0f));
	renderBox("underwaterBox", projection, view, glm::vec3(0, 0, 0));
	
	renderRoom1("room_1", projection, view, glm::vec3(0.9f, 0.9f, 0.8f)); //beige

	renderRoom2("room_2", projection, view, glm::vec3(2.0f, 0.6f, 0.8f)); //pink
	renderBallsToBounce("ball", projection, view, ballsAmount);

	renderRoom3("room_3", projection, view, glm::vec3(0.7f, 2.0f, 0.7f)); //green
	renderDust(projection, view, dustAmount, dustModelMatrices);

	renderRoom4("room_4", projection, view, glm::vec3(0.9f, 0.9f, 0.8f)); //beige

	renderRoom5("room_5", projection, view, glm::vec3(0.60f, 0.65f, 1.0f)); //blue
	renderBubbles(projection, view, bubblesAmount, bubblesModelMatrices);

	renderRoom6("room_6", projection, view, glm::vec3(1.6f, 1.0f, 0.6f)); //brown
	renderBoxesToShake("cont", projection, view, boxesAmount);
	
	renderWaterWaves(projection, view);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	applyBloom();

	renderScreen();

	glfwSwapBuffers(_window);
	glfwPollEvents();
}

void RenderSystem::initializeModels() {

	sphereModel = new Model("../models/sphere.obj");
	cubeModel = new Model("../models/cube.obj");
	dustModel = new Model("../models/rock.obj");
}

void RenderSystem::addShader(Shader *shader, string name) {
	Shaders[name] = shader;
}

void RenderSystem::addTexture(Texture2D *texture, string name) {
	Textures[name] = texture;
}

void RenderSystem::initializeShaders() {

	Shader *shader;

	shader = new Shader("LightShader.vert", "LightShader.frag");
	addShader(shader, "light");

	shader = new Shader("wind.vert", "wind.frag");
	addShader(shader, "wind");

	shader = new Shader("wave.vert", "wave.frag");
	addShader(shader, "wave");

	shader = new Shader("gaussianBlur.vert", "gaussianBlur.frag");
	addShader(shader, "blur");

	shader = new Shader("bloom_final.vert", "bloom_final.frag");
	addShader(shader, "bloomFinal");

	getShader("wind")->Use();
	getShader("wind")->setInt("texture_diffuse1", 0);

	getShader("blur")->Use();
	getShader("blur")->setInt("image", 0);

	getShader("bloomFinal")->Use();
	getShader("bloomFinal")->setInt("scene", 0);
	getShader("bloomFinal")->setInt("bloomBlur", 1);
}

void RenderSystem::initializeTextures() {

	Texture2D *texture;

	texture = new Texture2D("../textures/wall.jpg");
	addTexture(texture, "wall");

	texture = new Texture2D("../textures/container.png");
	addTexture(texture, "container");

	texture = new Texture2D("../textures/bouncing.jpg");
	addTexture(texture, "bouncing");

	texture = new Texture2D("../textures/wave.png");
	addTexture(texture, "wave");

	texture = new Texture2D("../textures/underwater.png");
	addTexture(texture, "underwater");

	texture = new Texture2D("../textures/bubble.png");
	addTexture(texture, "bubble");
}

void RenderSystem::initializeScreenQuad() {

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}

	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	glDrawBuffers(2, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	glGenFramebuffers(2, pingpongFBOs);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBOs[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}	
}

Shader* RenderSystem::getShader(string name) {

	if (Shaders.find(name) != Shaders.end())
		return Shaders[name];

	cout << "Shader Not Found" << endl;

	return nullptr;
}

Texture2D* RenderSystem::getTexture(string name) {

	if (Textures.find(name) != Textures.end())
		return Textures[name];

	cout << "Shader Not Found" << endl;

	return nullptr;
}

void RenderSystem::addRooms(string name) {

	_world->addRoom1(name + "_1", 20.0f, 20.0f, 20.0f, 0.0f, 0.0f, 0.0f); //base
	_world->addRoom2(name + "_2", 20.0f, 20.0f, 20.0f, -20.0f, 0.0f, 0.0f); //bouncing
	_world->addRoom3(name + "_3", 20.0f, 20.0f, 20.0f, 20.0f, 0.0f, 0.0f); //wind
	_world->addRoom4(name + "_4", 20.0f, 20.0f, 20.0f, 0.0f, 0.0f, 20.0f); //base
	_world->addRoom5(name + "_5", 20.0f, 20.0f, 20.0f, -20.0f, 0.0f, 20.0f); //underwater
	_world->addRoom6(name + "_6", 20.0f, 20.0f, 20.0f, 20.0f, 0.0f, 20.0f); //eartquake
}

void RenderSystem::addBallsToBounce(string name, unsigned int amount) {

	float maxX = -(11 - 0.0002f);
	float minX = -(28 + 0.0002f);
	float minY = (2 - 0.0002f);
	float maxY = (19 - 0.0002f);
	float minZ = (-18 - 0.0002f);
	float maxZ = (-1 + 0.0002f);

	for (unsigned int i = 0; i < amount; i++)
	{
		float x = minX + (((float)rand()) / (float)RAND_MAX) * (maxX - minX);
		float z = minZ + (((float)rand()) / (float)RAND_MAX) * (maxZ - minZ);
		float y = minY + (((float)rand()) / (float)RAND_MAX) * (maxY - minY);
		float rad = 0.1f + (((float)rand()) / (float)RAND_MAX) * (0.5f - 0.1f);
		
		_world->addSphere(name + to_string(i), rad, x, y, z, 1.0f);

		x = -20 + (((float)rand()) / (float)RAND_MAX) * (20 - -20);
		z = -10 + (((float)rand()) / (float)RAND_MAX) * (10 - -10);
		y = -20 + (((float)rand()) / (float)RAND_MAX) * (20 - -20);

		_world->getBody(name + to_string(i))->setLinearVelocity(btVector3(x, y, z));
		_world->getBody(name + to_string(i))->setRestitution(1.1f);
	}
}

void RenderSystem::addBoxesToShake(string name, unsigned int amount) {

	float maxX = (29 + 0.0002f);
	float minX = (13 - 0.0002f);
	float minZ = (2 - 0.0002f);
	float maxZ = (19 + 0.0002f);

	for (unsigned int i = 0; i < amount; i++)
	{
		float x = minX + (((float)rand()) / (float)RAND_MAX) * (maxX - minX);
		float z = minZ + (((float)rand()) / (float)RAND_MAX) * (maxZ - minZ);

		_world->addBox(name + to_string(i), 1, 1, 1, x, 5.0f, z, 10.0f);
		_world->getBody(name + to_string(i))->setAngularFactor(1);
	}
}

void RenderSystem::initializeDust() {

	dustModelMatrices = new glm::mat4[dustAmount];

	srand(glfwGetTime());

	float minX = (11 - 0.0002f);
	float maxX = (29 + 0.0002f);
	float minY = (0 + 0.0002f);
	float maxY = (19 - 0.0002f);
	float minZ = (-19 - 0.0002f);
	float maxZ = (-1 + 0.0002f);

	for (unsigned int i = 0; i < dustAmount; i++)
	{
		glm::mat4 model;

		float x = minX + (((float)rand()) / (float)RAND_MAX) * (maxX - minX);
		float z = minZ + (((float)rand()) / (float)RAND_MAX) * (maxZ - minZ);
		float y = minY + (((float)rand()) / (float)RAND_MAX) * (maxY - minY);

		model = glm::translate(model, glm::vec3(x, y, z));

		float scale = (rand() % 1) / 100.0f + 0.005f;
		model = glm::scale(model, glm::vec3(scale));

		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		dustModelMatrices[i] = model;
	}

	glGenBuffers(1, &dustVBO);
	glBindBuffer(GL_ARRAY_BUFFER, dustVBO);
	glBufferData(GL_ARRAY_BUFFER, dustAmount * sizeof(glm::mat4), &dustModelMatrices[0], GL_STREAM_DRAW);

	for (unsigned int i = 0; i < dustModel->meshes.size(); i++)
	{
		unsigned int VAO = dustModel->meshes[i].getVAO();
		glBindVertexArray(VAO);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}

void RenderSystem::initializeBubbles() {

	bubblesModelMatrices = new glm::mat4[bubblesAmount];
	srand(glfwGetTime());

	float maxX = (-11 + 0.0002f);
	float minX = (-29 - 0.0002f);
	float minY = (0 + 0.0002f);
	float maxY = (19 - 0.0002f);
	float maxZ = (19 + 0.0002f);
	float minZ = (1 - 0.0002f);

	for (unsigned int i = 0; i < bubblesAmount; i++)
	{
		glm::mat4 model;

		float x = minX + (((float)rand()) / (float)RAND_MAX) * (maxX - minX);
		float z = minZ + (((float)rand()) / (float)RAND_MAX) * (maxZ - minZ);
		float y = minY + (((float)rand()) / (float)RAND_MAX) * (maxY - minY);

		model = glm::translate(model, glm::vec3(x, y, z));

		float scale = (rand() % 5) / 100.0f + 0.01f;
		model = glm::scale(model, glm::vec3(scale));

		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		bubblesModelMatrices[i] = model;
	}

	glGenBuffers(1, &bubblesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, bubblesVBO);
	glBufferData(GL_ARRAY_BUFFER, bubblesAmount * sizeof(glm::mat4), &bubblesModelMatrices[0], GL_STREAM_DRAW);

	for (unsigned int i = 0; i < sphereModel->meshes.size(); i++)
	{
		unsigned int VAO = sphereModel->meshes[i].getVAO();
		glBindVertexArray(VAO);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}

void RenderSystem::renderRoom1(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color) {

	getShader("light")->Use();
	setupLightsParameter(color, glm::vec3(0.0f, 10.0f, -10.0f));

	getTexture("wall")->Bind();
	getShader("light")->setInt("material.diffuse", 0);

	glm::mat4 model(1.0);
	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);
	
	model = getBoxModelMatrix(_world->getWall(name + ".1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));
}

void RenderSystem::renderRoom2(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color) {

	getShader("light")->Use();
	setupLightsParameter(color, glm::vec3(-20.0f, 10.0f, -10.0f));

	getTexture("wall")->Bind();

	glm::mat4 model(1.0);
	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);
	
	model = getBoxModelMatrix(_world->getWall(name + ".1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));
}

void RenderSystem::renderRoom3(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color) {

	getShader("light")->Use();
	setupLightsParameter(color, glm::vec3(20.0f, 10.0f, -10.0f));

	getTexture("wall")->Bind();

	glm::mat4 model(1.0);
	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);
	
	model = getBoxModelMatrix(_world->getWall(name + ".1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));
}

void RenderSystem::renderRoom4(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color) {

	getShader("light")->Use();
	setupLightsParameter(color, glm::vec3(0.0f, 10.0f, 10.0f));

	getTexture("wall")->Bind();

	glm::mat4 model(1.0);
	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);
	
	model = getBoxModelMatrix(_world->getWall(name + ".1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));
	
	model = getBoxModelMatrix(_world->getWall(name + ".5.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));
}

void RenderSystem::renderRoom5(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color) {
	
	getShader("light")->Use();
	setupLightsParameter(color, glm::vec3(-20.0f, 10.0f, 10.0f));

	getTexture("underwater")->Bind();

	glm::mat4 model(1.0);
	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);

	model = getBoxModelMatrix(_world->getWall(name + ".1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));
}

void RenderSystem::renderRoom6(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color) {

	getShader("light")->Use();
	setupLightsParameter(color, glm::vec3(20.0f, 10.0f, 10.0f));

	getTexture("wall")->Bind();

	glm::mat4 model(1.0);
	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);
	
	model = getBoxModelMatrix(_world->getWall(name + ".1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".4.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));
	
	model = getBoxModelMatrix(_world->getWall(name + ".5.1"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.2"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".5.3"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));

	model = getBoxModelMatrix(_world->getWall(name + ".6"));
	getShader("light")->setMat4("modelMatrix", model);
	cubeModel->Draw(getShader("light"));
}

void RenderSystem::renderBox(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color) {
	
	getShader("light")->Use();
	getTexture("container")->Bind();

	glm::mat4 model = getBoxModelMatrix(_world->getBody(name));

	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);
	getShader("light")->setMat4("modelMatrix", model);

	cubeModel->Draw(getShader("light"));
}

void RenderSystem::renderSphere(string name, glm::mat4 projection, glm::mat4 view, glm::vec3 color) {

	getShader("light")->Use();
	getTexture("bouncing")->Bind();

	glm::mat4 model = getSphereModelMatrix(_world->getBody(name));

	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);
	getShader("light")->setMat4("modelMatrix", model);

	sphereModel->Draw(getShader("light"));
}

void RenderSystem::renderBallsToBounce(string name, glm::mat4 projection, glm::mat4 view, unsigned int amount) {
	
	getShader("light")->Use();
	getTexture("bouncing")->Bind();

	setupLightsParameter(glm::vec3(1.0f, 0.6f, 0.8f), glm::vec3(-20.0f, 10.0f, -10.0f));
	
	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);
	
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = getSphereModelMatrix(_world->getBody(name + to_string(i)));

		getShader("light")->setMat4("modelMatrix", model);

		sphereModel->Draw(getShader("light"));
	}
}

void RenderSystem::renderBoxesToShake(string name, glm::mat4 projection, glm::mat4 view, unsigned int amount) {
	
	getShader("light")->Use();
	getTexture("container")->Bind();

	setupLightsParameter(glm::vec3(1.6f, 1.0f, 0.6f), glm::vec3(20.0f, 10.0f, 10.0f));

	getShader("light")->setMat4("projectionMatrix", projection);
	getShader("light")->setMat4("viewMatrix", view);
	

	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = getBoxModelMatrix(_world->getBody(name + to_string(i)));

		getShader("light")->setMat4("modelMatrix", model);

		cubeModel->Draw(getShader("light"));
	}
}

void RenderSystem::renderDust(glm::mat4 projection, glm::mat4 view, unsigned int amount, glm::mat4* modelMatrices) {

	for (unsigned int i = 0; i < amount; i++)
	{
		if ((modelMatrices[i][3][0] > (10 - 0.0002f) && modelMatrices[i][3][0] < (30 + 0.0002f)) && modelMatrices[i][3][2] > (-20 - 0.0002f) && modelMatrices[i][3][2] < (0 + 0.0002f)) {
			modelMatrices[i][3][0] += 0.03f;
			modelMatrices[i][3][2] -= 0.03f;
		}
		else {
			modelMatrices[i][3][0] = (10) + (((float)rand()) / (float)RAND_MAX) * ((30 + 0.0002f) - (10 - 0.0002f));
			modelMatrices[i][3][2] = (-20 - 0.0002f) + (((float)rand()) / (float)RAND_MAX) * ((0 + 0.0002f) - (-20 - 0.0002f));
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, dustVBO);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &modelMatrices[0]);

	getShader("wind")->Use();

	getShader("wind")->setMat4("projection", projection);
	getShader("wind")->setMat4("view", view);
	getShader("wind")->setVec4("color", glm::vec4(1, 1, 0, 1));
	getShader("wind")->setFloat("mixRatio", 0.8f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dustModel->textures_loaded[0].id);
	for (unsigned int i = 0; i < dustModel->meshes.size(); i++)
	{
		glBindVertexArray(dustModel->meshes[i].getVAO());
		glDrawElementsInstanced(GL_TRIANGLES, dustModel->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
		glBindVertexArray(0);
	}
}

void RenderSystem::renderWaterWaves(glm::mat4 projection, glm::mat4 view) {
	
	getShader("wave")->Use();
	getTexture("wave")->Bind();

	getShader("wave")->setMat4("projectionMatrix", projection);
	getShader("wave")->setMat4("viewMatrix", view);

	glm::mat4 model = glm::mat4(1.0);

	model = glm::translate(model, glm::vec3(-20, 10, 10));
	model = glm::scale(model, glm::vec3(10, 10, 10));

	getShader("wave")->setMat4("modelMatrix", model);

	glm::vec2 texCoord;
	float decimal;
	texCoord.x = -modf(glfwGetTime() * 0.1f, &decimal);
	texCoord.y = 1;

	getShader("wave")->setVec2("uvRotation", texCoord);

	cubeModel->Draw(getShader("wave"));
}

void RenderSystem::renderBubbles(glm::mat4 projection, glm::mat4 view, unsigned int amount, glm::mat4* modelMatrices) {

	for (unsigned int i = 0; i < amount; i++)
	{
		if ( modelMatrices[i][3][1] < (20 - 0.0002f)) {
			modelMatrices[i][3][1] += 0.03f;
		}
		else {
			modelMatrices[i][3][1] = (0 + 0.0002f) + (((float)rand()) / (float)RAND_MAX) * ((20 - 0.0002f) - (0 + 0.0002f));
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, bubblesVBO);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, amount * sizeof(glm::mat4), &modelMatrices[0]);

	getShader("wind")->Use();
	getTexture("bubble")->Bind();

	getShader("wind")->setMat4("projection", projection);
	getShader("wind")->setMat4("view", view);
	getShader("wind")->setVec4("color", glm::vec4(0.40f, 0.4f, 1.0f, 0.5f));
	getShader("wind")->setFloat("mixRatio", 0.5f);
	
	for (unsigned int i = 0; i < sphereModel->meshes.size(); i++)
	{
		glBindVertexArray(sphereModel->meshes[i].getVAO());
		glDrawElementsInstanced(GL_TRIANGLES, sphereModel->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
		glBindVertexArray(0);
	}
}

void RenderSystem::renderScreen() {

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

}

void RenderSystem::setupLightsParameter(glm::vec3 ambientColor, glm::vec3 pointPosition) {

	getShader("light")->Use();

	getShader("light")->setInt("material.diffuse", 0);
	getShader("light")->setVec3("viewPos", _camera->Position);
	getShader("light")->setFloat("material.shininess", 64.0f);

	// directional light
	//getShader("light")->setVec3("dirLight.direction", 0.0f, 1.0f, 0.0f);
	//getShader("light")->setVec3("dirLight.ambient", -ambientColor.x*0, -ambientColor.y*0, -ambientColor.z*0);
	//getShader("light")->setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
	//getShader("light")->setVec3("dirLight.diffuse", 0.2f, 0.2f, 0.2f);
	//getShader("light")->setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);

	// point light 1
	getShader("light")->setVec3("pointLights[0].position", glm::vec3(pointPosition.x, pointPosition.y, pointPosition.z));
	getShader("light")->setVec3("pointLights[0].ambient", ambientColor.x, ambientColor.y, ambientColor.z);
	getShader("light")->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
	getShader("light")->setVec3("pointLights[0].specular", 0.5f, 0.5f, 0.5f);
	getShader("light")->setFloat("pointLights[0].constant", 1.0f);
	getShader("light")->setFloat("pointLights[0].linear", 0.09f);
	getShader("light")->setFloat("pointLights[0].quadratic", 0.032f);

	// point light 2
	getShader("light")->setVec3("pointLights[1].position", glm::vec3(pointPosition.x - abs(pointPosition.x*0.25f), pointPosition.y, pointPosition.z /*- abs(pointPosition.z*0.25f)*/));
	getShader("light")->setVec3("pointLights[1].ambient", ambientColor.x, ambientColor.y, ambientColor.z);
	getShader("light")->setVec3("pointLights[1].diffuse",  0.8f, 0.8f, 0.8f);
	getShader("light")->setVec3("pointLights[1].specular", 0.5f, 0.5f, 0.5f);
	getShader("light")->setFloat("pointLights[1].constant", 1.0f);
	getShader("light")->setFloat("pointLights[1].linear", 0.09f);
	getShader("light")->setFloat("pointLights[1].quadratic", 0.032f);
}

void RenderSystem::applyBloom() {

	getShader("blur")->Use();

	bool horizontal = true, first_iteration = true;
	unsigned int amount = 10;

	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBOs[horizontal]);
		getShader("blur")->setInt("horizontal", horizontal);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
		renderScreen();
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	getShader("bloomFinal")->Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

	if ((_world->getBody("player")->getWorldTransform().getOrigin().getX() < (-9.75f) && _world->getBody("player")->getWorldTransform().getOrigin().getX() > (-29.3f)) && (_world->getBody("player")->getWorldTransform().getOrigin().getZ() > (-0.08f) && _world->getBody("player")->getWorldTransform().getOrigin().getZ() < (19.3f))) {
		if (!bloom)
			bloom = true;
	}
	else
	{
		if (bloom)
			bloom = false;		
	}

	getShader("bloomFinal")->setBool("bloom", bloom);
	getShader("bloomFinal")->setFloat("exposure", exposure);

	//getShader("bloomFinal")->setBool("shake", true);
	//getShader("bloomFinal")->setFloat("time", glfwGetTime());

	//cout << "bloom: " << (bloom ? "on" : "off") << "| exposure: " << exposure << endl;
}

void RenderSystem::applyWind() {

	map<string, btRigidBody*> bodies= _world->getBodies();

	for (map<string, btRigidBody*>::iterator it = bodies.begin(); it != bodies.end(); ++it)
	{ 
		if (((*it).second->getWorldTransform().getOrigin().getX() > (11-0.0002) && (*it).second->getWorldTransform().getOrigin().getX() < (29 + 0.0002)) && ((*it).second->getWorldTransform().getOrigin().getZ() > (-19 - 0.0002) && (*it).second->getWorldTransform().getOrigin().getZ() < (-1 + 0.0002))) {

			(*it).second->activate();
			(*it).second->applyCentralForce(btVector3(40, 30, -40));
		}			
	}
	bodies.clear();
}

void RenderSystem::applyEathquake() {

	map<string, btRigidBody*> bodies = _world->getBodies();

	for (map<string, btRigidBody*>::iterator it = bodies.begin(); it != bodies.end(); ++it)
	{
		if (((*it).second->getWorldTransform().getOrigin().getX() > (11 - 0.0002) && (*it).second->getWorldTransform().getOrigin().getX() < (29 + 0.0002)) && ((*it).second->getWorldTransform().getOrigin().getZ() > (1 - 0.0002) && (*it).second->getWorldTransform().getOrigin().getZ() < (19 + 0.0002))) {
			(*it).second->activate();

			int numManifolds = _world->getWorld()->getDispatcher()->getNumManifolds();
			for (int i = 0; i < numManifolds; i++)
			{
				btPersistentManifold* contactManifold = _world->getWorld()->getDispatcher()->getManifoldByIndexInternal(i);
				const btCollisionObject* obA = contactManifold->getBody0();
				const btCollisionObject* obB = contactManifold->getBody1();

				int numContacts = contactManifold->getNumContacts();
				for (int j = 0; j < numContacts; j++)
				{
					btManifoldPoint& pt = contactManifold->getContactPoint(j);
					if (pt.getDistance() < 0.f)
					{
						const btVector3& ptA = pt.getPositionWorldOnA();
						const btVector3& ptB = pt.getPositionWorldOnB();
						const btVector3& normalOnB = pt.m_normalWorldOnB;

						if (obB == (*it).second && obA == _world->getWall("room_6.1")) {

							float x = -0.2f+ (((float)rand()) / (float)RAND_MAX) * (0.2f - -0.2f);
							float z = -0.2f+ (((float)rand()) / (float)RAND_MAX) * (0.2f - -0.2f);
							float y = -0.2f+ (((float)rand()) / (float)RAND_MAX) * (0.2f - -0.2f);
							
							(*it).second->applyCentralImpulse(btVector3(x, y, z)*20.0f);
						}
						if (obA == (*it).second && obB == _world->getWall("room_6.1")) {
							float x = -0.2f + (((float)rand()) / (float)RAND_MAX) * (0.2f - -0.2f);
							float z = -0.2f + (((float)rand()) / (float)RAND_MAX) * (0.2f - -0.2f);
							float y = -0.2f + (((float)rand()) / (float)RAND_MAX) * (0.2f - -0.2f);

							//if(obA == _world->getBody("player"))
							//(*it).second->applyImpulse(btVector3(x, y, z), ptA);
							(*it).second->applyCentralImpulse(btVector3(x, y, z)*60.0f);
						}
					}
				}
			}
		}
	}
	bodies.clear();
}

void RenderSystem::applyUnderwater() {

	map<string, btRigidBody*> bodies = _world->getBodies();

	for (map<string, btRigidBody*>::iterator it = bodies.begin(); it != bodies.end(); ++it)
	{
		if (((*it).second->getWorldTransform().getOrigin().getX() < (-11 - 0.0002) && (*it).second->getWorldTransform().getOrigin().getX() > (-29 + 0.0002)) && ((*it).second->getWorldTransform().getOrigin().getZ() > (1 - 0.0002) && (*it).second->getWorldTransform().getOrigin().getZ() < (19 + 0.0002))) {
			
			(*it).second->activate();
			(*it).second->setGravity(btVector3(0,-5,0));
			(*it).second->setFriction(3);
			if ((*it).first == "player") {
				_camera->MaxSpeed = 5.0f;
			}
		}
		else
		{		
			(*it).second->activate();
			if((*it).second->getGravity().getY() == -5)
				(*it).second->setGravity(btVector3(0, -10, 0));
			if ((*it).second->getFriction() == 3) {
				(*it).second->setFriction(1);
				if ((*it).first == "player") {
					_camera->MaxSpeed = 10.0f;
				}
			}
		}
	}
	bodies.clear();
}