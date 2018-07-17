#include "Camera.h"

Camera* Camera::camera = nullptr;

Camera::Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, 1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	_world = &BulletWorld::getBulletWorld();
	_world->addBox("player", 0.5f, 1.0f, 0.5f, 0.0, 2.0, -15.0, 10);
	_world->getBody("player")->setFriction(1.0);
	_world->getBody("player")->setAngularFactor(btVector3(0, 0, 0));
	_world->getBody("player")->setRestitution(0.9f);

	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	MaxSpeed = 10.0f;
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	_world = &BulletWorld::getBulletWorld();
	Position = glm::vec3(posX, posY, posZ);
	WorldUp = glm::vec3(upX, upY, upZ);
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

Camera::~Camera(){}

Camera& Camera::getCamera()
{
	if (camera == nullptr)
		camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));

	return *camera;
}

glm::mat4 Camera::GetViewMatrix() {
	return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
	float velocity = MovementSpeed * deltaTime;

	if (direction == FORWARD) {
		_world->getBody("player")->activate();
		_world->getBody("player")->applyForce(btVector3(Front.x, Front.y, Front.z) * velocity, btVector3(0.0, 0.48, -0.4));
	}		
	if (direction == BACKWARD) {
		_world->getBody("player")->activate();
		_world->getBody("player")->applyForce(-btVector3(Front.x, Front.y, Front.z) * velocity, btVector3(0.0, 0.48, -0.4));
	}
	if (direction == LEFT) {
		_world->getBody("player")->activate();
		_world->getBody("player")->applyForce(-btVector3(Right.x, Right.y, Right.z) * velocity, btVector3(0.0, 0.48, -0.4));
	}
	if (direction == RIGHT) {
		_world->getBody("player")->activate();
		_world->getBody("player")->applyForce(btVector3(Right.x, Right.y, Right.z) * velocity, btVector3(0.0, 0.48, -0.4));
	}
	if (direction == UPWARD && _world->getBody("player")->getWorldTransform().getOrigin().getY() <= 1.0f) {
		_world->getBody("player")->activate();
		_world->getBody("player")->applyCentralImpulse(btVector3(0, 30, 0));
	}

	btVector3 currVelocity = _world->getBody("player")->getLinearVelocity();
	btScalar speed = currVelocity.length();

	if (speed > MaxSpeed) {
		currVelocity *= MaxSpeed / speed;
		_world->getBody("player")->setLinearVelocity(currVelocity);
	}

	Position.y = 0.0f;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {

	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw = glm::mod(Yaw + xoffset, 360.0f);
	Pitch += yoffset;

	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
	if (Zoom >= 1.0f && Zoom <= 45.0f)
		Zoom -= yoffset;
	if (Zoom <= 1.0f)
		Zoom = 1.0f;
	if (Zoom >= 45.0f)
		Zoom = 45.0f;
}

void Camera::updateCameraVectors() {

		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);

		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
}