#pragma once

#define GLFW_DLL

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <btBulletDynamicsCommon.h>

glm::mat4 getSphereModelMatrix(btRigidBody* sphere);
glm::mat4 getPlaneModelMatrix(btRigidBody* plane);
glm::mat4 getBoxModelMatrix(btRigidBody* box);
