#pragma once
#include "Module.h"
#include "Entity.h"
#include "box2d/box2d.h"
#include <list>

#define GRAVITY_X 0.0f
#define GRAVITY_Y -10.0f

#define PIXELS_PER_METER 50.0f // if touched change METER_PER_PIXEL too
#define METER_PER_PIXEL 0.02f // this is 1 / PIXELS_PER_METER !

#define METERS_TO_PIXELS(m) ((int) floor(PIXELS_PER_METER * m))
#define PIXEL_TO_METERS(p)  ((float) METER_PER_PIXEL * p)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

// types of bodies
enum bodyType {
	DYNAMIC,
	STATIC,
	KINEMATIC
};

enum class ColliderType {
	PLAYER,
	ITEM,
	PLATFORM,
	PLATFORMICE,
	TERRESTRE,
	AMEGO,
	VOLADOR,
	ENEMY,
	PLAYER_ATTACK,
	PLAYER_WHIP_ATTACK,
	DIALOGOS,
	SENSOR,
	ASCENSORES,
	ABYSS,
	ASCENSOR,
	GIRO,
	BOSS,
	BOSS_ATTACK,
	CARONTE,
	ATTACKSENSOR,
	DOORS,
	LEVER,
	PROJECTILE,
	BUFON,
	BUFON_JUMP_ATTACK_AREA,
	BUFON_ATTACK,
	MOSAIC_PIECE,
	MOSAIC_LEVER,
	CHECKPOINT,
	DEVIL,
	DEVIL_PUNCH_ATTACK1,
	DEVIL_JUMP_ATTACK2,
	ESTATUAS,
	DEVIL_TAIL_ATTACK,SPEAR,
	BOSSFLOOR,
	UNKNOWN
	// ..
};

// Small class to return to other modules to track position and rotation of physics bodies
class PhysBody
{
public:
	PhysBody() : listener(NULL), body(NULL), ctype(ColliderType::UNKNOWN)
	{}

	~PhysBody() {}

	void GetPosition(int& x, int& y) const;
	float GetRotation() const;
	bool Contains(int x, int y) const;
	int RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const;

public:
	int width = 0;
	int height = 0;
	bool Salio;
	std::string ID;
	std::string sensorID;
	b2Body* body;
	Entity* listener;
	ColliderType ctype;
	
};

// Module --------------------------------------
class Physics : public Module, public b2ContactListener // TODO
{
public:

	// Constructors & Destructors
	Physics();
	~Physics();

	// Main module steps
	bool Start();
	bool PreUpdate();
	bool PostUpdate();
	bool CleanUp();

	// Create basic physics objects
	PhysBody* CreateRectangle(int x, int y, int width, int height, bodyType type);
	PhysBody* CreateCircle(int x, int y, int radious, bodyType type);
	PhysBody* CreateRectangleSensor(int x, int y, int width, int height, bodyType type);
	PhysBody* CreateChain(int x, int y, int* points, int size, bodyType type);
	
	// b2ContactListener ---
	void BeginContact(b2Contact* contact);
	void EndContact(b2Contact* contact);

	void DeletePhysBody(PhysBody* physBody);
	bool IsPendingToDelete(PhysBody* physBody);

	bool IsDebug() const { return debug; }
	b2World* world;
private:

	// Debug mode
	bool debug;


	// List of physics bodies
	std::list<PhysBody*> bodiesToDelete;
};