#include "BulletWorld.h"

BulletWorld* BulletWorld::bulletWorld = nullptr;

BulletWorld::BulletWorld(glm::vec3 gravity) {

	collisionConfig = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfig);
	broadphase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
	world->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
	world->getSolverInfo().m_splitImpulse = true;
}

BulletWorld::~BulletWorld() {
	for (map<string, btRigidBody*>::iterator it = bodies.begin(); it != bodies.end(); ++it)
	{
		world->removeCollisionObject((*it).second);
		btMotionState* motionState = ((*it)).second->getMotionState();
		btCollisionShape* shape = ((*it)).second->getCollisionShape();
		delete ((*it).second);
		delete shape;
		delete motionState;
	}

	for (map<string, btRigidBody*>::iterator it = walls.begin(); it != walls.end(); ++it)
	{
		world->removeCollisionObject((*it).second);
		btMotionState* motionState = ((*it)).second->getMotionState();
		btCollisionShape* shape = ((*it)).second->getCollisionShape();
		delete ((*it).second);
		delete shape;
		delete motionState;
	}

	delete dispatcher;
	delete collisionConfig;
	delete solver;
	delete broadphase;
	delete world;
}

BulletWorld& BulletWorld::getBulletWorld() {

	if (bulletWorld == nullptr) {
		bulletWorld = new BulletWorld(glm::vec3(0.0f, -10.0f, 0.0f));
	}

	return *bulletWorld;
}

btRigidBody* BulletWorld::addSphere(string name, float rad, float x, float y, float z, float mass) {
	
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(x, y, z));

	btSphereShape* sphere = new btSphereShape(rad);

	btVector3 inertia(0, 0, 0);
	if (mass != 0.0)
		sphere->calculateLocalInertia(mass, inertia);

	btMotionState* motion = new btDefaultMotionState(t);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motion, sphere, inertia);
	btRigidBody* body = new btRigidBody(info);

	world->addRigidBody(body);
	bodies[name] = body;

	return body;
}

btRigidBody* BulletWorld::addFloor(string name, glm::vec3 origin_location, glm::vec3 floor_normal, float planeconstant) {
	
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(origin_location.x, origin_location.y, origin_location.z));

	btStaticPlaneShape* plane = new btStaticPlaneShape(btVector3(floor_normal.x, floor_normal.y, floor_normal.z), (btScalar)planeconstant);
	
	btMotionState* motion = new btDefaultMotionState(t);
	btRigidBody::btRigidBodyConstructionInfo info(0.0, motion, plane, btVector3(origin_location.x, origin_location.y, origin_location.z));
	btRigidBody* body = new btRigidBody(info);
	
	world->addRigidBody(body);
	bodies[name] = body;

	return body;
}

btRigidBody* BulletWorld::addBox(string name, float width, float height, float depth, float x, float y, float z, float mass) {
	
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(x, y, z));

	btBoxShape* box = new btBoxShape(btVector3(width *0.5f, height *0.5f, depth *0.5f));

	btVector3 inertia(0, 0, 0);
	if (mass != 0.0)
		box->calculateLocalInertia(mass, inertia);

	btMotionState* motion = new btDefaultMotionState(t);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motion, box, inertia);
	btRigidBody* body = new btRigidBody(info);
	
	world->addRigidBody(body);
	bodies[name] = body;

	return body;
}

btRigidBody* BulletWorld::addWall(string name, float width, float height, float depth, float x, float y, float z) {
	
	float mass = 10.0f;
	
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(x, y, z));
	
	btBoxShape* box = new btBoxShape(btVector3(width * 0.5f, height * 0.5f, depth * 0.5f));

	btVector3 inertia(0, 0, 0);
	if (mass != 0.0)
		box->calculateLocalInertia(mass, inertia);

	btMotionState* motion = new btDefaultMotionState(t);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motion, box, inertia);
	btRigidBody* body = new btRigidBody(info);
	body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
	body->setActivationState(DISABLE_DEACTIVATION);
	body->setLinearFactor(btVector3(0, 0, 0));
	body->setAngularFactor(btVector3(0, 0, 0));

	world->addRigidBody(body);
	walls[name] = body;

	return body;
}

btRigidBody* BulletWorld::addFrontWallWithDoor(string name, float width, float height, float depth, float x, float y, float z, float doorWidth, float doorHeight) {
	
	btRigidBody* center = addWall(name + ".1", doorWidth, height - doorHeight - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x, y + (height * 0.5f) + (doorHeight * 0.5f), z - HALF_WALL_THICKNESS);
	btRigidBody* left = addWall(name + ".2", (width * 0.5f) - (doorWidth * 0.5f), height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x - ((width * 0.25f) + (doorWidth * 0.25f)), y + (height * 0.5f), z - HALF_WALL_THICKNESS);
	btRigidBody* right = addWall(name + ".3", (width * 0.5f) - (doorWidth * 0.5f), height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x + ((width * 0.25f) + (doorWidth * 0.25f)), y + (height * 0.5f), z - HALF_WALL_THICKNESS);

	return nullptr;
}

btRigidBody* BulletWorld::addBackWallWithDoor(string name, float width, float height, float depth, float x, float y, float z, float doorWidth, float doorHeight) {
	
	btRigidBody* center = addWall(name + ".1", doorWidth, height - doorHeight - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x, y + (height * 0.5f) + (doorHeight * 0.5f), z - depth + HALF_WALL_THICKNESS);
	btRigidBody* left = addWall(name + ".2", (width * 0.5f) - (doorWidth * 0.5f), height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x - ((width * 0.25f) + (doorWidth * 0.25f)), y + (height * 0.5f), z - depth + HALF_WALL_THICKNESS);
	btRigidBody* right = addWall(name + ".3", (width * 0.5f) - (doorWidth * 0.5f), height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x + ((width * 0.25f) + (doorWidth * 0.25f)), y + (height * 0.5f), z - depth + HALF_WALL_THICKNESS);

	return nullptr;
}

btRigidBody* BulletWorld::addLeftWallWithDoor(string name, float width, float height, float depth, float x, float y, float z, float doorWidth, float doorHeight) {
	
	btRigidBody* center = addWall(name + ".1", WALL_THICKNESS, height - doorHeight - DOUBLE_WALL_THICKNESS, doorWidth, x - (width * 0.5f) + HALF_WALL_THICKNESS, y + (height * 0.5f) + (doorHeight * 0.5f), z - (depth * 0.5f));
	btRigidBody* left = addWall(name + ".2", WALL_THICKNESS, height - DOUBLE_WALL_THICKNESS, (depth * 0.5f) - (doorWidth * 0.5f) - WALL_THICKNESS, x - (width * 0.5f) + HALF_WALL_THICKNESS, y + (height * 0.5f), z - ((depth * 0.25f) - (doorWidth * 0.25f)) - HALF_WALL_THICKNESS);
	btRigidBody* right = addWall(name + ".3", WALL_THICKNESS, height - DOUBLE_WALL_THICKNESS, (depth * 0.5f) - (doorWidth * 0.5f) - WALL_THICKNESS, x - (width * 0.5f) + HALF_WALL_THICKNESS, y + (height * 0.5f), z - ((depth * 0.75f) + (doorWidth * 0.25f)) + HALF_WALL_THICKNESS);

	return nullptr;
}

btRigidBody* BulletWorld::addRightWallWithDoor(string name, float width, float height, float depth, float x, float y, float z, float doorWidth, float doorHeight) {

	btRigidBody* center = addWall(name + ".1", WALL_THICKNESS, height - doorHeight - DOUBLE_WALL_THICKNESS, doorWidth, x + (width * 0.5f) - HALF_WALL_THICKNESS, y + (height * 0.5f) + (doorHeight * 0.5f), z - (depth * 0.5f));
	btRigidBody* left = addWall(name + ".2", WALL_THICKNESS, height - DOUBLE_WALL_THICKNESS, (depth * 0.5f) - (doorWidth * 0.5f) - WALL_THICKNESS, x + (width * 0.5f) - HALF_WALL_THICKNESS, y + (height * 0.5f), z - ((depth * 0.25f) - (doorWidth * 0.25f)) - HALF_WALL_THICKNESS);
	btRigidBody* right = addWall(name + ".3", WALL_THICKNESS, height - DOUBLE_WALL_THICKNESS, (depth * 0.5f) - (doorWidth * 0.5f) - WALL_THICKNESS, x + (width * 0.5f) - HALF_WALL_THICKNESS, y + (height * 0.5f), z - ((depth * 0.75f) + (doorWidth * 0.25f)) + HALF_WALL_THICKNESS);

	return nullptr;
}

btRigidBody* BulletWorld::addRoom1(string name, float width, float height, float depth, float x, float y, float z) {

	btRigidBody* floor = addWall(name + ".1", width, WALL_THICKNESS, depth, x, y + HALF_WALL_THICKNESS, z - (depth * 0.5f));
	btRigidBody* ceiling = addWall(name + ".2", width, WALL_THICKNESS, depth, x, y + height - HALF_WALL_THICKNESS, z - (depth * 0.5f));

	addFrontWallWithDoor(name+".3",  width,  height,  depth,  x,  y,  z, DOOR_WIDTH, DOOW_HEIGHT);
	btRigidBody* back = addWall(name + ".4", width, height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x , y + (height * 0.5f), z - depth + HALF_WALL_THICKNESS);

	addLeftWallWithDoor(name + ".5", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);
	addRightWallWithDoor(name + ".6", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);

	return nullptr;
}

btRigidBody* BulletWorld::addRoom2(string name, float width, float height, float depth, float x, float y, float z) {

	btRigidBody* floor = addWall(name + ".1", width, WALL_THICKNESS, depth, x, y + HALF_WALL_THICKNESS, z - (depth * 0.5f));
	btRigidBody* ceiling = addWall(name + ".2", width, WALL_THICKNESS, depth, x, y + height - HALF_WALL_THICKNESS, z - (depth * 0.5f));

	addFrontWallWithDoor(name + ".3", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);
	btRigidBody* back = addWall(name + ".4", width, height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x , y + (height * 0.5f), z - depth + HALF_WALL_THICKNESS);

	btRigidBody* left = addWall(name + ".5", WALL_THICKNESS, height - DOUBLE_WALL_THICKNESS, depth - DOUBLE_WALL_THICKNESS, x - (width * 0.5f) + HALF_WALL_THICKNESS, y + (height * 0.5f), z - (depth * 0.5f));
	addRightWallWithDoor(name + ".6", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);

	floor->setRestitution(1.0f);
	ceiling->setRestitution(1.0f);

	getWall(name  +".3.1")->setRestitution(1.0f);
	getWall(name + ".3.2")->setRestitution(1.0f);
	getWall(name + ".3.3")->setRestitution(1.0f);
	back->setRestitution(1.0f);

	left->setRestitution(1.0f);
	getWall(name + ".6.1")->setRestitution(1.0f);
	getWall(name + ".6.2")->setRestitution(1.0f);
	getWall(name + ".6.3")->setRestitution(1.0f);

	return nullptr;
}

btRigidBody* BulletWorld::addRoom3(string name, float width, float height, float depth, float x, float y, float z) {

	btRigidBody* floor = addWall(name + ".1", width, WALL_THICKNESS, depth, x, y + HALF_WALL_THICKNESS, z - (depth * 0.5f));
	btRigidBody* ceiling = addWall(name + ".2", width, WALL_THICKNESS, depth, x, y + height - HALF_WALL_THICKNESS, z - (depth * 0.5f));
	
	addFrontWallWithDoor(name + ".3", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);
	btRigidBody* back = addWall(name + ".4", width, height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x , y + (height * 0.5f), z - depth + HALF_WALL_THICKNESS);

	addLeftWallWithDoor(name + ".5", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);
	btRigidBody* right = addWall(name + ".6", WALL_THICKNESS, height - DOUBLE_WALL_THICKNESS, depth - DOUBLE_WALL_THICKNESS, x + (width * 0.5f) - HALF_WALL_THICKNESS, y + (height * 0.5f), z - (depth * 0.5f));

	return nullptr;
}

btRigidBody* BulletWorld::addRoom4(string name, float width, float height, float depth, float x, float y, float z) {

	btRigidBody* floor = addWall(name + ".1", width, WALL_THICKNESS, depth, x, y + HALF_WALL_THICKNESS, z - (depth * 0.5f));
	btRigidBody* ceiling = addWall(name + ".2", width, WALL_THICKNESS, depth, x, y + height - HALF_WALL_THICKNESS, z - (depth * 0.5f));
	
	btRigidBody* front = addWall(name + ".3", width, height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x, y + (height * 0.5f), z - HALF_WALL_THICKNESS);
	addBackWallWithDoor(name + ".4", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);

	addLeftWallWithDoor(name + ".5", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);
	addRightWallWithDoor(name + ".6", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);

	return nullptr;
}

btRigidBody* BulletWorld::addRoom5(string name, float width, float height, float depth, float x, float y, float z) {

	btRigidBody* floor = addWall(name + ".1", width, WALL_THICKNESS, depth, x, y + HALF_WALL_THICKNESS, z - (depth * 0.5f));
	btRigidBody* ceiling = addWall(name + ".2", width, WALL_THICKNESS, depth, x, y + height - HALF_WALL_THICKNESS, z - (depth * 0.5f));
	
	btRigidBody* front = addWall(name + ".3", width, height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x, y + (height * 0.5f), z - HALF_WALL_THICKNESS);
	addBackWallWithDoor(name + ".4", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);

	btRigidBody* left = addWall(name + ".5", WALL_THICKNESS, height - DOUBLE_WALL_THICKNESS, depth - DOUBLE_WALL_THICKNESS, x - (width * 0.5f) + HALF_WALL_THICKNESS, y + (height * 0.5f), z - (depth * 0.5f));
	addRightWallWithDoor(name + ".6", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);

	return nullptr;
}

btRigidBody* BulletWorld::addRoom6(string name, float width, float height, float depth, float x, float y, float z) {

	btRigidBody* floor = addWall(name + ".1", width, WALL_THICKNESS, depth, x, y + HALF_WALL_THICKNESS, z - (depth * 0.5f));
	btRigidBody* ceiling = addWall(name + ".2", width, WALL_THICKNESS, depth, x, y + height - HALF_WALL_THICKNESS, z - (depth * 0.5f));
	
	btRigidBody* front = addWall(name + ".3", width, height - DOUBLE_WALL_THICKNESS, WALL_THICKNESS, x, y + (height * 0.5f), z - HALF_WALL_THICKNESS);
	addBackWallWithDoor(name + ".4", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);

	addLeftWallWithDoor(name + ".5", width, height, depth, x, y, z, DOOR_WIDTH, DOOW_HEIGHT);
	btRigidBody* right = addWall(name + ".6", WALL_THICKNESS, height - DOUBLE_WALL_THICKNESS, depth - DOUBLE_WALL_THICKNESS, x + (width * 0.5f) - HALF_WALL_THICKNESS, y + (height * 0.5f), z - (depth * 0.5f));

	return nullptr;
}

btDiscreteDynamicsWorld* BulletWorld::getWorld() {
	return world;
}

map<string, btRigidBody*> BulletWorld::getBodies() {
	return bodies;
}

map<string, btRigidBody*> BulletWorld::getRooms() {
	return walls;
}

btRigidBody* BulletWorld::getBody(string name) {

	if (bodies.find(name) != bodies.end())
		return bodies[name];

	cout << "Body Not Found" << endl;

	return nullptr;
}

btRigidBody* BulletWorld::getWall(string name) {

	if (walls.find(name) != walls.end())
		return walls[name];

	cout << "Wall Not Found" << endl;

	return nullptr;
}
