#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

// Actor

class Actor: public GraphObject
{
public:
	Actor(StudentWorld* world, int imageID, bool visible, int startX, int startY, Direction dir, double size, unsigned int depth);
	virtual ~Actor();

	virtual void doSomething() = 0;

	// Mutators
	void setDead();
	virtual bool annoy(unsigned int amt);
	bool moveToLocation(int x, int y);
	bool moveOneStepInDirection(Direction dir);

	// Accessors
	StudentWorld* getWorld() const;

	// Identifiers
	bool isAlive() const;
	virtual bool blocks() const;
	virtual bool isDigger() const;
	virtual bool huntsFrackMan() const;

private:
	StudentWorld* m_world;
	bool m_alive;
};

// Dirt

class Dirt : public Actor
{
public:
	Dirt(StudentWorld* world, int startX, int startY);
	virtual ~Dirt();

	virtual void doSomething();
};

class LivingActor : public Actor
{
public:
	LivingActor(StudentWorld* world, int imageID, int startX, int startY, int health, Direction dir);
	virtual ~LivingActor();

	// Mutators
	virtual bool annoy(unsigned int amt);
	virtual void addGold() = 0;

	// Accessors
	int getHealth() const;
private:
	virtual void giveUp() = 0;
	int m_health;
};


// FrackMan

class FrackMan: public LivingActor
{
public:
	FrackMan(StudentWorld* world);
	virtual ~FrackMan();

	virtual void doSomething();

	// Mutators
	virtual void addGold();
	void addSonar();
	void addWater();

	// Accessors
	unsigned int getGold() const;
	unsigned int getSonar() const;
	unsigned int getWater() const;

	// Identifiers
	virtual bool isDigger() const;

private:
	virtual void giveUp();
	int m_nGold;
	int m_nSonar;
	int m_nWater;
};

// Protester

class Protester : public LivingActor
{
public:
	Protester(StudentWorld* world, int imageID = IID_PROTESTER, unsigned int health = 5, int score = 100);
	virtual ~Protester();

	virtual void doSomething();

	// Mutators
	virtual void addGold();
	virtual bool annoy(unsigned int amt);

	// Identifiers
	virtual bool huntsFrackMan() const;

protected:
	void setTicksUntilNextMove(int ticks);	// Mutator
private:
	virtual void giveUp();
	Direction randomDirection() const;
	Direction choosePerpendicular(Direction dir);
	virtual bool useTechnology();
	int m_score;
	int m_stepsForward;
	int m_ticksBetweenMoves;
	int m_restTicks;
	int m_perpendicularTicks;
	int m_shoutTicks;
	int m_stunTicks;
	bool m_leavingState;
};

// HardcoreProtester

class HardCoreProtester : public Protester
{
public:
	HardCoreProtester(StudentWorld* world);
	virtual ~HardCoreProtester();

	// Mutators
	virtual void addGold();
private:
	virtual bool useTechnology();

};

// Boulder

class Boulder : public Actor
{
public:
	Boulder(StudentWorld* world, int startX, int startY);
	virtual ~Boulder();

	virtual void doSomething();

	// Identifiers
	virtual bool blocks() const;
private:
	int m_state;
	int m_waitingTicks;
};

// Squirt

class Squirt : public Actor
{
public:
	Squirt(StudentWorld* world, int startX, int startY, Direction dir);
	virtual ~Squirt();

	virtual void doSomething();

private:
	int m_travelDist;
};

// Goodies

class Goodie : public Actor
{
public:
	Goodie::Goodie(StudentWorld* world, int imageID, bool visible, int startX, int startY);
	virtual ~Goodie();

	virtual void doSomething();

protected:
	// Mutators
	void setTicksToLive(unsigned int ticks);
	void decreaseTick();
private:
	int m_ticks;
	virtual void soundPickUp() const;
	virtual void addPoints() = 0;
};

class Barrel : public Goodie
{
public:
	Barrel(StudentWorld* world, int startX, int startY);
	virtual ~Barrel();

private:
	virtual void soundPickUp() const;
	virtual void addPoints();
};

class Gold : public Goodie
{
public:
	Gold(StudentWorld* world, int startX, int startY, bool tempState);
	virtual ~Gold();

	virtual void doSomething();
private:
	virtual void addPoints();
	bool m_tempState;
};

class SonarKit : public Goodie
{
public:
	SonarKit(StudentWorld* world);
	virtual ~SonarKit();

	virtual void doSomething();
private:
	virtual void addPoints();
};

class WaterPool : public Goodie
{
public:
	WaterPool(StudentWorld* world, int startX, int startY);
	virtual ~WaterPool();
	virtual void doSomething();
private:
	virtual void addPoints();
};

#endif // ACTOR_H_
