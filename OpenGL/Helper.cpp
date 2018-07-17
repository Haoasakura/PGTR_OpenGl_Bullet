#include "Helper.h"

glm::mat4 getSphereModelMatrix(btRigidBody* sphere)
{
	float r = ((btSphereShape*)sphere->getCollisionShape())->getRadius();
	
	btTransform t;
	sphere->getMotionState()->getWorldTransform(t);
	
	float mat[16];
	glm::mat4 ret(1.0);

	t.getOpenGLMatrix(mat);
	ret = glm::scale(ret, glm::vec3(r, r, r));
	ret = glm::make_mat4(mat) * ret;

	return ret;
}

glm::mat4 getPlaneModelMatrix(btRigidBody* plane)
{
	btTransform t;
	plane->getMotionState()->getWorldTransform(t);
	
	float mat[16];
	glm::mat4 ret(1.0);

	t.getOpenGLMatrix(mat);
	ret = glm::make_mat4(mat);

	return ret;
}

glm::mat4 getBoxModelMatrix(btRigidBody* box)
{
	btVector3 extent = ((btBoxShape*)box->getCollisionShape())->getHalfExtentsWithMargin();
	btTransform t;
	box->getMotionState()->getWorldTransform(t);
	
	float mat[16];
	glm::mat4 ret(1.0);

	t.getOpenGLMatrix(mat);
	ret = glm::scale(ret, glm::vec3(extent.x(), extent.y(), extent.z()));
	ret = glm::make_mat4(mat) * ret;

	return ret;
}
