#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GraphObject.h"
#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>

class Actor;
class FrackMan;
class Dirt;

int randInt(int min, int max);

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir);

	virtual ~StudentWorld();

	// Stages of the game
	virtual int init();
	virtual int move();
	virtual void cleanUp();

	// Mutators
	void addActor(Actor* a);
	void revealNearbyObjects(int x, int y, double r);
	void addGoodie();
	void decreaseBarrels();
	void removeDeadActors();
	bool removeDirt(int x, int y);
	int annoyActors(Actor* a, int points, double r);

	bool bribeNearbyEnemy(Actor* a, double r);
	void decreaseNumEnemies();

	void fireSquirt();

	void annoyFrackMan();
	void giveFrackManGold();
	void giveFrackManSonar();
	void giveFrackManWater();

	GraphObject::Direction determineFirstMoveToExit(int x, int y);
	GraphObject::Direction determineFirstMoveToFrackMan(int x, int y, int maxSteps);

	// Checking functions
	bool canMoveTo(Actor* a, int x, int y) const;
	bool canMoveInDirection(Actor* a, GraphObject::Direction dir, int r = 1) const;
	bool nearFrackMan(Actor* a, double r) const;
	bool facingTowardFrackMan(Actor* a) const;
	GraphObject::Direction lineOfSightToFrackMan(Actor* a) const;

private:
	class Coord
	{
	public:
		Coord(int rr, int cc) : m_r(rr), m_c(cc) {}
		int r() const { return m_r; }
		int c() const { return m_c; }
	private:
		int m_r;
		int m_c;
	};

	FrackMan* m_player;
	Dirt* m_dirt[VIEW_HEIGHT][VIEW_WIDTH];
	std::vector<Actor*> actors;
	int m_nBarrels;

	int m_ticksBetweenProtesters;
	int m_ticks;
	int m_numProtesters;
	int m_targetNumProtesters;
	int m_probabilityHardcore;

	int m_mazeExit[VIEW_HEIGHT - 3][VIEW_WIDTH - 3];
	int m_mazeFrackMan[VIEW_HEIGHT - 3][VIEW_WIDTH - 3];

	void setDisplayText();
	std::string formatScore(int score, int level, int lives, int health, int water, int gold, int sonar, int barrels) const;
	std::string formatStat(int stat, int digits, char leadChar = ' ') const;

	bool finishedLevel() const;

	bool withinRadius(int x1, int y1, int x2, int y2, double r) const;
	bool dirtLess(int x, int y) const;
	bool inDirt(int x, int y) const;
	void pickLocation(int& x, int& y, int lowerx = 0, int upperx = VIEW_WIDTH - 4, int lowery = 20, int uppery = 56);
	void pickValidLocation(int& x, int& y);
	void locationInFront(Actor* a, int r, int& x, int& y);
	double distance(int x1, int y1, int x2, int y2) const;
	void updateMaze(int maze[][VIEW_WIDTH - 3], int ex, int ey);
	GraphObject::Direction determineFirstMove(int maze[][VIEW_WIDTH - 3], int x, int y, int maxSteps = VIEW_WIDTH * VIEW_HEIGHT);
};

#endif // STUDENTWORLD_H_
