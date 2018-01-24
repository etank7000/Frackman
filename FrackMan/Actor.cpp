#include "Actor.h"
#include "StudentWorld.h"
#include <algorithm>
using namespace std;

// CONSTANTS

const int STABLE = 0;
const int WAITING = 1;
const int FALLING = 2;

//////////////////////////////
// ACTOR
//////////////////////////////

Actor::Actor(StudentWorld* world, int imageID, bool visible, int startX, int startY, Direction dir, double size, unsigned int depth)
	:GraphObject(imageID, startX, startY, dir, size, depth)
{
	m_world = world;
	setVisible(visible);
	m_alive = true;
}

Actor::~Actor()
{

}

// Set the Actor's state to dead
void Actor::setDead()
{
	m_alive = false;
}

// Many actors cannot be annoyed.
bool Actor::annoy(unsigned int amt)
{
	return false;
}

// Moves an actor to (x, y) if it can and return true. Return false otherwise.
bool Actor::moveToLocation(int x, int y)
{
	if (getWorld()->canMoveTo(this, x, y))
	{
		moveTo(x, y);
		return true;
	}
	return false;
}

// Moves an actor one step in Direction dir. Return true if moved, false otherwise.
bool Actor::moveOneStepInDirection(Direction dir)
{
	switch (dir)
	{
	case right:
		return moveToLocation(getX() + 1, getY());
	case left:
		return moveToLocation(getX() - 1, getY());
	case up:
		return moveToLocation(getX(), getY() + 1);
	case down:
		return moveToLocation(getX(), getY() - 1);
	}
	return false;
}

// Return a pointer to the StudentWorld member variable.
StudentWorld* Actor::getWorld() const
{
	return m_world;
}

// Is the Actor alive?
bool Actor::isAlive() const
{
	return m_alive;
}

// Does this Actor block others from passing through it?
bool Actor::blocks() const
{
	return false;
}

// Can this Actor dig dirt?
bool Actor::isDigger() const
{
	return false;
}

// Does this Actor hunt the FrackMan?
bool Actor::huntsFrackMan() const
{
	return false;
}

///////////////////////////////
// DIRT
///////////////////////////////

Dirt::Dirt(StudentWorld* world, int startX, int startY)
	:Actor(world, IID_DIRT, true, startX, startY, right, 0.25, 3)
{

}

Dirt::~Dirt()
{

}

void Dirt::doSomething()
{
	// Dirt doesn't do anything...
}

////////////////////////////////////
// Living Actor
///////////////////////////////////

LivingActor::LivingActor(StudentWorld* world, int imageID, int startX, int startY, int health, Direction dir)
	:Actor(world, imageID, true, startX, startY, dir, 1.0, 0)
{
	m_health = health;
}
LivingActor::~LivingActor()
{

}

// Deals amt points of damage to the Actor and makes them give up if their health falls to or below zero.
bool LivingActor::annoy(unsigned int amt)
{
	m_health -= amt;
	if (m_health <= 0)
		giveUp();
	return true;
}

int LivingActor::getHealth() const
{
	return m_health;
}


/////////////////////////////////
// FRACKMAN
/////////////////////////////////

FrackMan::FrackMan(StudentWorld* world)
	:LivingActor(world, IID_PLAYER, 30, 60, 10, right)
{
	m_nGold = 0;
	m_nSonar = 1;
	m_nWater = 5;
}

FrackMan::~FrackMan()
{

}

void FrackMan::doSomething()
{
	if (!isAlive())
		return;
	int ch;
	if (getWorld()->getKey(ch))
	{
		// user hit a key this tick!
		switch (ch)
		{
		case KEY_PRESS_ESCAPE:
			setDead();
			break;
		case KEY_PRESS_SPACE:
			if (m_nWater > 0)
			{
				getWorld()->fireSquirt();
				m_nWater--;
			}

			break;
		case KEY_PRESS_LEFT:
			if (getDirection() != left)
				setDirection(left);
			else if (getX() == 0)
				moveTo(getX(), getY());
			else
				moveToLocation(getX() - 1, getY());
			break;
		case KEY_PRESS_RIGHT:
			if (getDirection() != right)
				setDirection(right);
			else if (getX() == 60)
				moveTo(getX(), getY());
			else
				moveToLocation(getX() + 1, getY());
			break;
		case KEY_PRESS_DOWN:
			if (getDirection() != down)
				setDirection(down);
			else if (getY() == 0)
				moveTo(getX(), getY());
			else
				moveToLocation(getX(), getY() - 1);
			break;
		case KEY_PRESS_UP:
			if (getDirection() != up)
				setDirection(up);
			else if (getY() == 60)
				moveTo(getX(), getY());
			else
				moveToLocation(getX(), getY() + 1);
			break;
		case 'z': case 'Z':
			getWorld()->revealNearbyObjects(getX(), getY(), 12.0);
			getWorld()->playSound(SOUND_SONAR);
			m_nSonar--;
			break;
		case KEY_PRESS_TAB:
			if (m_nGold > 0)
			{
				getWorld()->addActor(new Gold(getWorld(), getX(), getY(), true));
				m_nGold--;
			}
			break;
		}
	}
	if (getWorld()->removeDirt(getX(), getY()))
		getWorld()->playSound(SOUND_DIG);
	getWorld()->revealNearbyObjects(getX(), getY(), 4.0);
}

// Add a gold nugget to FrackMan's inventory.
void FrackMan::addGold()
{
	m_nGold++;
}

// Add a sonar kit to FrackMan's inventory.
void FrackMan::addSonar()
{
	m_nSonar++;
}

// Add 5 squirts of water to FrackMan's inventory.
void FrackMan::addWater()
{
	m_nWater += 5;
}

unsigned int FrackMan::getGold() const
{
	return m_nGold;
}

unsigned int FrackMan::getSonar() const
{
	return m_nSonar;
}

unsigned int FrackMan::getWater() const
{
	return m_nWater;
}

// The FrackMan can dig through dirt.
bool FrackMan::isDigger() const
{
	return true;
}

void FrackMan::giveUp()
{
	getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
	setDead();
}

///////////////////////////////////
// Protester
///////////////////////////////////

Protester::Protester(StudentWorld* world, int imageID, unsigned int health, int score)
	:LivingActor(world, imageID, 60, 60, health, left)
{
	m_score = score;
	m_stepsForward = randInt(8, 60);
	m_ticksBetweenMoves = max(0, 3 - (int)getWorld()->getLevel() / 4);
	m_restTicks = 0;
	m_perpendicularTicks = 0;
	m_shoutTicks = 0;
	m_stunTicks = max(50, 100 - (int)getWorld()->getLevel() * 10);
	m_leavingState = false;
}

Protester::~Protester()
{

}

void Protester::doSomething()
{
	if (!isAlive())		// If protester isn't alive, return
		return;

	// If protester is in a "rest state" during current tick, update resting tick count and return
	if (m_restTicks > 0)
	{
		m_restTicks--;
		return;
	}
	setTicksUntilNextMove(m_ticksBetweenMoves);

	// Leave the oil field state
	if (m_leavingState)
	{
		int x = getX();
		int y = getY();
		if (x == 60 && y == 60)		// Set dead if reached destination
		{
			setDead();
			getWorld()->decreaseNumEnemies();
		}
		else		// Otherwise, move one square closer to exit
		{
			Direction toMove = getWorld()->determineFirstMoveToExit(x, y);
			setDirection(toMove);
			moveOneStepInDirection(toMove);
		}
		return;
	}

	if (m_perpendicularTicks < 200)
		m_perpendicularTicks++;

	// Otherwise if within 4 units of FrackMan and facing in FrackMan's direction and not shouted within non-resting 15 ticks:
	bool isNearFrackMan = getWorld()->nearFrackMan(this, 4.0);
	if (isNearFrackMan && getWorld()->facingTowardFrackMan(this) && m_shoutTicks == 0)
	{
		m_shoutTicks = 15;
		getWorld()->playSound(SOUND_PROTESTER_YELL);
		getWorld()->annoyFrackMan();
		return;
	}
	else if (m_shoutTicks > 0)
	{
		m_shoutTicks--;
		return;
	}

	// Otherwise if more than 4 units away and successfully uses technology, return
	if (!isNearFrackMan && useTechnology())
		return;

	// Otherwise if straight line sight and more than 4 units away:
	Direction toFrackMan = getWorld()->lineOfSightToFrackMan(this);
	if (toFrackMan != none && !isNearFrackMan)
	{
		setDirection(toFrackMan);
		moveOneStepInDirection(getDirection());
		m_stepsForward = 0;
		return;
	}


	// Otherwise if can't see FrackMan:
	m_stepsForward--;
	Direction perpendicularDir = choosePerpendicular(getDirection());
	if (m_stepsForward <= 0)
	{
		Direction randDir;
		do
		{
			randDir = randomDirection();
		} while (!getWorld()->canMoveInDirection(this, randDir));
		setDirection(randDir);
		m_stepsForward = randInt(8, 60);
	}
	else if (m_perpendicularTicks == 200 && perpendicularDir != none)
	{
		setDirection(perpendicularDir);
		m_stepsForward = randInt(8, 60);
		m_perpendicularTicks = 0;
	}
	if (!moveOneStepInDirection(getDirection()))
		m_stepsForward = 0;
}

// Regular Protesters leave the oil field after picking up one gold nugget
void Protester::addGold()
{
	getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	getWorld()->increaseScore(25);
	m_leavingState = true;
	setTicksUntilNextMove(0);
}

// Protesters get stunned if annoyed. If their health falls below zero, they are set to a leave the oil field state.
bool Protester::annoy(unsigned int amt)
{
	if (m_leavingState)
		return false;
	LivingActor::annoy(amt);
	if (getHealth() > 0)
	{
		getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
		setTicksUntilNextMove(m_stunTicks);
	}
	else if (amt == 100)	// Gave up by boulder
		getWorld()->increaseScore(500);
	else if (amt == 2)		// Gave up by squirt
		getWorld()->increaseScore(m_score);
	return true;
}

// Protesters hunt the FrackMan by shouting at them.
bool Protester::huntsFrackMan() const
{
	return true;
}

// Stuns the Protester for a certain amount of game ticks.
void Protester::setTicksUntilNextMove(int ticks)
{
	m_restTicks = ticks;
}

// Protesters are set to leave-the-oil-field state if they give up.
void Protester::giveUp()
{
	m_leavingState = true;
	m_restTicks = 0;
	getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
}

// Chooses a random direction for the Protester to walk.
GraphObject::Direction Protester::randomDirection() const
{
	int random = randInt(1, 4);
	switch (random)
	{
	case 1:
		return right;
	case 2:
		return left;
	case 3:
		return up;
	case 4:
		return down;
	}
	return none;
}

// Chooses a perpendicular direction from Direction dir.
GraphObject::Direction Protester::choosePerpendicular(Direction dir)
{
	switch (dir)
	{
	case right: case left:
		if (getWorld()->canMoveInDirection(this, up))
			return up;
		else if (getWorld()->canMoveInDirection(this, down))
			return down;
		break;
	case up: case down:
		if (getWorld()->canMoveInDirection(this, left))
			return left;
		else if (getWorld()->canMoveInDirection(this, right))
			return right;
		break;
	}
	return none;
}

// Regular Protesters do not use their cell phone to track the FrackMan.
bool Protester::useTechnology()
{
	return false;
}

////////////////////////////////////////
// HARDCORE PROTESTER
////////////////////////////////////////

HardCoreProtester::HardCoreProtester(StudentWorld* world)
	:Protester(world, IID_HARD_CORE_PROTESTER, 20, 250)
{

}

HardCoreProtester::~HardCoreProtester()
{

}

void HardCoreProtester::addGold()
{
	getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
	getWorld()->increaseScore(50);
	int ticksToStare = max(50, 100 - (int)getWorld()->getLevel() * 10);
	setTicksUntilNextMove(ticksToStare);
}

// Hardcore Protesters use their cell phone to track where the FrackMan is.
bool HardCoreProtester::useTechnology()
{
	int M = 16 + (int)getWorld()->getLevel() * 2;
	Direction toFrackMan = getWorld()->determineFirstMoveToFrackMan(getX(), getY(), M);
	if (toFrackMan != none)
	{
		setDirection(toFrackMan);
		moveOneStepInDirection(toFrackMan);
		return true;
	}
	return false;
}

///////////////////////
// BOULDER
///////////////////////

Boulder::Boulder(StudentWorld* world, int startX, int startY)
	:Actor(world, IID_BOULDER, true, startX, startY, down, 1.0, 1)
{
	m_state = STABLE;
	getWorld()->removeDirt(startX, startY);
	m_waitingTicks = 0;
}

Boulder::~Boulder()
{

}

void Boulder::doSomething()
{
	if (!isAlive())
		return;
	switch (m_state)
	{
	case STABLE:
		if (getWorld()->canMoveTo(this, getX(), getY() - 1))
			m_state = WAITING;
		break;
	case WAITING:
		if (m_waitingTicks == 30)
		{
			m_state = FALLING;
			getWorld()->playSound(SOUND_FALLING_ROCK);
		}
		else
			m_waitingTicks++;
		break;
	case FALLING:
		if (!moveToLocation(getX(), getY() - 1))	// Crashed into dirt or another boulder
		{
			setDead();
			break;
		}
		getWorld()->annoyActors(this, 100, 3.0);
		break;
	}
}

// Boulders prevent other actors from passing through.
bool Boulder::blocks() const
{
	return true;
}

//////////////////////////
// SQUIRT
//////////////////////////

Squirt::Squirt(StudentWorld* world, int startX, int startY, Direction dir)
	:Actor(world, IID_WATER_SPURT, true, startX, startY, dir, 1.0, 1)
{
	m_travelDist = 4;
}

Squirt::~Squirt()
{

}

void Squirt::doSomething()
{
	if (!isAlive())
		return;
	if (getWorld()->annoyActors(this, 2, 3.0) > 0 || m_travelDist <= 0 || !getWorld()->canMoveInDirection(this, getDirection()))
		setDead();
	else
	{
		moveOneStepInDirection(getDirection());
		m_travelDist--;
	}
}

/////////////////////////
// GOODIE
/////////////////////////

Goodie::Goodie(StudentWorld* world, int imageID, bool visible, int startX, int startY)
	:Actor(world, imageID, visible, startX, startY, right, 1.0, 2)
{

}

Goodie::~Goodie()
{

}

void Goodie::doSomething()
{
	if (!isAlive())
		return;
	if (getWorld()->nearFrackMan(this, 3.0))
	{
		setDead();
		soundPickUp();
		addPoints();
	}
}

// The Goodie will disappear after a certain amount of ticks.
void Goodie::setTicksToLive(unsigned int ticks)
{
	m_ticks = ticks;
}

// Decrease the Goodie's lifespan by one tick.
void Goodie::decreaseTick()
{
	m_ticks--;
	if (m_ticks == 0)
		setDead();
}

void Goodie::soundPickUp() const
{
	getWorld()->playSound(SOUND_GOT_GOODIE);
}

/////////////////////////
// BARREL
/////////////////////////

Barrel::Barrel(StudentWorld* world, int startX, int startY)
	:Goodie(world, IID_BARREL, false, startX, startY)
{

}

Barrel::~Barrel()
{

}

void Barrel::soundPickUp() const
{
	getWorld()->decreaseBarrels();
	getWorld()->playSound(SOUND_FOUND_OIL);
}

void Barrel::addPoints()
{
	getWorld()->increaseScore(1000);	// Barrel worth 1000 points
}

////////////////////////////
// GOLD
////////////////////////////

Gold::Gold(StudentWorld* world, int startX, int startY, bool tempState)
	:Goodie(world, IID_GOLD, false, startX, startY)
{
	m_tempState = tempState;
	if (tempState)
		setTicksToLive(100);
}

Gold::~Gold()
{

}

void Gold::doSomething()
{
	if (!isAlive())
		return;
	if (!m_tempState)
		Goodie::doSomething();
	else
	{
		decreaseTick();
		if (getWorld()->bribeNearbyEnemy(this, 3.0))
			setDead();
	}
}

void Gold::addPoints()
{
	getWorld()->increaseScore(10);	// Gold worth 10 points
	getWorld()->giveFrackManGold();
}

/////////////////////////////////
// SONAR KIT
/////////////////////////////////

SonarKit::SonarKit(StudentWorld* world)
	:Goodie(world, IID_SONAR, true, 0, 60)
{
	int ticks = max(100, 300 - 10 * (int)getWorld()->getLevel());
	setTicksToLive(ticks);
}

SonarKit::~SonarKit()
{

}

void SonarKit::addPoints()
{
	getWorld()->increaseScore(75);	// Sonar Kit worth 75 points
	getWorld()->giveFrackManSonar();
}

void SonarKit::doSomething()
{
	Goodie::doSomething();
	decreaseTick();
}

//////////////////////////////////////
// WATER POOL
//////////////////////////////////////

WaterPool::WaterPool(StudentWorld* world, int startX, int startY)
	:
	Goodie(world, IID_WATER_POOL, true, startX, startY)
{
	int ticks = max(100, 300 - 10 * (int)getWorld()->getLevel());
	setTicksToLive(ticks);
}

WaterPool::~WaterPool()
{

}

void WaterPool::addPoints()
{
	getWorld()->increaseScore(100);	// Water Pool worth 100 points
	getWorld()->giveFrackManWater();
}

void WaterPool::doSomething()
{
	Goodie::doSomething();
	decreaseTick();
}
