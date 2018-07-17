#pragma once

#include <map>
#include <list>
#include <glm\glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <btBulletDynamicsCommon.h>

#include "Helper.h"

using namespace std;

class BulletWorld
{
private:

	const float WALL_THICKNESS=0.5f;
	const float HALF_WALL_THICKNESS = WALL_THICKNESS * 0.5f;
	const float DOUBLE_WALL_THICKNESS = WALL_THICKNESS * 2.0f;
	const float DOOR_WIDTH = 2.0f;
	const float DOOW_HEIGHT = 2.5f;

	static BulletWorld *bulletWorld;

	btDiscreteDynamicsWorld* world;
	btDispatcher* dispatcher;
	btCollisionConfiguration* collisionConfig;
	btBroadphaseInterface* broadphase;
	btConstraintSolver* solver;

	map<string, btRigidBody*> bodies;
	map<string, btRigidBody*> walls;

public:

	static BulletWorld& getBulletWorld();

	map<string, btRigidBody*> getBodies();
	map<string, btRigidBody*> getRooms();
	btDiscreteDynamicsWorld* getWorld();
	btRigidBody* getBody(string name);
	btRigidBody* getWall(string name);

	BulletWorld(glm::vec3 gravity);
	~BulletWorld();

	btRigidBody* addFloor(string name, glm::vec3 origin_location, glm::vec3 floor_normal, float plane_constant);
	btRigidBody* addSphere(string name, float rad, float x, float y, float z, float mass);
	btRigidBody* addBox(string name, float width, float height, float depth, float x, float y, float z, float mass);
	btRigidBody* addWall(string name, float width, float height, float depth, float x, float y, float z);

	btRigidBody* addFrontWallWithDoor(string name, float width, float height, float depth, float x, float y, float z, float doorWidth, float doorHeight);
	btRigidBody* addBackWallWithDoor(string name, float width, float height, float depth, float x, float y, float z, float doorWidth, float doorHeight);
	btRigidBody* addLeftWallWithDoor(string name, float width, float height, float depth, float x, float y, float z, float doorWidth, float doorHeight);
	btRigidBody* addRightWallWithDoor(string name, float width, float height, float depth, float x, float y, float z, float doorWidth, float doorHeight);
	
	btRigidBody* addRoom1(string name, float width, float height, float depth, float x, float y, float z);
	btRigidBody* addRoom2(string name, float width, float height, float depth, float x, float y, float z);
	btRigidBody* addRoom3(string name, float width, float height, float depth, float x, float y, float z);
	btRigidBody* addRoom4(string name, float width, float height, float depth, float x, float y, float z);
	btRigidBody* addRoom5(string name, float width, float height, float depth, float x, float y, float z);
	btRigidBody* addRoom6(string name, float width, float height, float depth, float x, float y, float z);

	void stepSimulate()
	{
		world->stepSimulation(btScalar(1.0 / 60.0));
	}

};
