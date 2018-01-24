#include "StudentWorld.h"
#include "Actor.h"
#include <string>
#include <algorithm>
#include <random>
#include <queue>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(std::string assetDir)
	: GameWorld(assetDir)
{
	m_player = nullptr;
	for (int i = 0; i < VIEW_HEIGHT - 4; i++)
		for (int j = 0; j < VIEW_WIDTH; j++)
			m_dirt[i][j] = nullptr;
}

StudentWorld::~StudentWorld()
{
	delete m_player;
	for (int i = 0; i < VIEW_HEIGHT - 4; i++)
		for (int j = 0; j < VIEW_WIDTH; j++)
			delete m_dirt[i][j];
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end();)
	{
		delete *it;
		it = actors.erase(it);
	}
}

int StudentWorld::init()
{
	m_ticksBetweenProtesters = max(25, 200 - (int)getLevel());
	m_ticks = m_ticksBetweenProtesters;
	m_numProtesters = 0;
	m_targetNumProtesters = min(15, 2 + (int)((int)getLevel() * 1.5));
	m_probabilityHardcore = min(90, (int)getLevel() * 10 + 30);

	m_player = new FrackMan(this);	// Create the FrackMan
	for (int i = 0; i < VIEW_HEIGHT - 4; i++)	// Create the Dirt
		for (int j = 0; j < VIEW_WIDTH; j++)
		{
			if (j >= 30 && j <= 33 && i >= 4 && i <= 59)	// Vertical mine shaft
				m_dirt[i][j] = nullptr;
			else
				m_dirt[i][j] = new Dirt(this, j, i);
		}

	int B = min((int)getLevel() / 2 + 2, 6);	// Number of boulders
	for (int i = 0; i < B; i++)
	{
		int x, y;
		pickValidLocation(x, y);
		addActor(new Boulder(this, x, y));
	}

	int G = max(5 - (int)getLevel() / 2, 2);	// Number of gold nuggets

	for (int i = 0; i < G; i++)
	{
		int x, y;
		pickValidLocation(x, y);
		addActor(new Gold(this, x, y, false));
	}

	m_nBarrels = min(2 + (int)getLevel(), 20);			// Number of barrels

	for (int i = 0; i < m_nBarrels; i++)
	{
		int x, y;
		pickValidLocation(x, y);
		addActor(new Barrel(this, x, y));
	}

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	// This code is here merely to allow the game to build, run, and terminate after you hit enter a few times.
	// Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
	//decLives();
	setDisplayText();
	if (m_ticks == m_ticksBetweenProtesters && m_numProtesters < m_targetNumProtesters)
	{
		int rand = randInt(1, 100);
		if (rand <= m_probabilityHardcore)
			addActor(new HardCoreProtester(this));
		else
			addActor(new Protester(this));
		m_numProtesters++;
		m_ticks = 0;
	}
	else if (m_ticks < m_ticksBetweenProtesters)
		m_ticks++;
	addGoodie();
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++)
	{
		if ((*it)->isAlive())
		{
			(*it)->doSomething();
			if (!m_player->isAlive())
			{
				decLives();
				return GWSTATUS_PLAYER_DIED;
			}

			if (finishedLevel())
				return GWSTATUS_FINISHED_LEVEL;
		}
	}
	m_player->doSomething();
	if (!m_player->isAlive())
	{
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	if (finishedLevel())
		return GWSTATUS_FINISHED_LEVEL;
	removeDeadActors();
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	delete m_player;
	for (int i = 0; i < VIEW_HEIGHT - 4; i++)
		for (int j = 0; j < VIEW_WIDTH; j++)
			delete m_dirt[i][j];
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end();)
	{
		delete *it;
		it = actors.erase(it);
	}
}

// Add an Actor to the world
void StudentWorld::addActor(Actor* a)
{
	actors.push_back(a);
}

// Reveal all objects within radius r of (x, y)
void StudentWorld::revealNearbyObjects(int x, int y, double r)
{
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++)
	{
		if (withinRadius(x, y, (*it)->getX(), (*it)->getY(), r))
			(*it)->setVisible(true);
	}
}

// Chance to add a Sonar Kit or Water Pool
void StudentWorld::addGoodie()
{
	int G = getLevel() * 25 + 300;
	int rand = randInt(1, G);
	if (rand == 1)	// 1 is an arbitrary choice
	{
		int rand2 = randInt(1, 5);
		if (rand2 == 1)
			addActor(new SonarKit(this));
		else
		{
			int x, y;
			do
			{
				pickLocation(x, y, 0, VIEW_WIDTH - 4, 0, VIEW_HEIGHT - 4);
			} while (!dirtLess(x, y));	// Make sure to pick a location (x, y) whose 4x4 square is free of dirt
			addActor(new WaterPool(this, x, y));
		}
	}
}

// Reduce the number of oil barrels FrackMan needs to find to finish the level.
void StudentWorld::decreaseBarrels()
{
	m_nBarrels--;
}

// Remove all actors who are not alive anymore.
void StudentWorld::removeDeadActors()
{
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end();)
	{
		if (!(*it)->isAlive())
		{
			delete *it;
			it = actors.erase(it);
		}
		else
			it++;
	}
}

// Remove a 4x4 square of dirt at (x, y)
bool StudentWorld::removeDirt(int x, int y)
{
	bool removed = false;
	for (int i = y; i < VIEW_HEIGHT - 4 && i <= y + 3; i++)
		for (int j = x; j < VIEW_WIDTH && j <= x + 3; j++)
			if (m_dirt[i][j] != nullptr)
			{
				delete m_dirt[i][j];
				m_dirt[i][j] = nullptr;
				removed = true;
			}
	return removed;
}

// Deal some points of annoyance to actors within a radius of r.
int StudentWorld::annoyActors(Actor* a, int points, double r)
{
	int nAnnoyed = 0;
	if (withinRadius(a->getX(), a->getY(), m_player->getX(), m_player->getY(), r) && m_player->annoy(points))
		nAnnoyed++;
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++)
		if (withinRadius(a->getX(), a->getY(), (*it)->getX(), (*it)->getY(), r) && (*it)->annoy(points))
			nAnnoyed++;
	return nAnnoyed;
}

// Bribes nearby enemies with gold.
bool StudentWorld::bribeNearbyEnemy(Actor* a, double r)
{
	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++)
		if ((*it)->huntsFrackMan() && withinRadius(a->getX(), a->getY(), (*it)->getX(), (*it)->getY(), r))
		{
			Protester* p = dynamic_cast<Protester*>(*it);
			p->addGold();
			return true;
		}
	return false;
}

void StudentWorld::decreaseNumEnemies()
{
	m_numProtesters--;
}

void StudentWorld::fireSquirt()
{
	playSound(SOUND_PLAYER_SQUIRT);
	int x, y;
	locationInFront(m_player, 4, x, y);
	if (dirtLess(x, y) && canMoveTo(m_player, x, y))
		addActor(new Squirt(this, x, y, m_player->getDirection()));
}

void StudentWorld::annoyFrackMan()
{
	m_player->annoy(2);
}

void StudentWorld::giveFrackManGold()
{
	m_player->addGold();
}

void StudentWorld::giveFrackManSonar()
{
	m_player->addSonar();
}

void StudentWorld::giveFrackManWater()
{
	m_player->addWater();
}

// Return the direction for the optimal first move toward the exit (60, 60).
GraphObject::Direction StudentWorld::determineFirstMoveToExit(int x, int y)
{
	updateMaze(m_mazeExit, 60, 60);
	return determineFirstMove(m_mazeExit, x, y);
}

// Return the direction for the optimal move toward the FrackMan.
GraphObject::Direction StudentWorld::determineFirstMoveToFrackMan(int x, int y, int maxSteps)
{
	updateMaze(m_mazeFrackMan, m_player->getX(), m_player->getY());
	return determineFirstMove(m_mazeFrackMan, x, y, maxSteps);
}

// Return whether an Actor can move to (x, y).
bool StudentWorld::canMoveTo(Actor* a, int x, int y) const
{
	if (!dirtLess(x, y) && !a->isDigger())
		return false;
	for (vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++)
	{
		if ( (*it != a && (*it)->blocks() && withinRadius(x, y, (*it)->getX(), (*it)->getY(), 3.0))
			|| (x < 0 || x > VIEW_WIDTH - 4 || y < 0 || y > VIEW_HEIGHT - 4) )
			return false;
	}
	return true;
}

// Return whether Actor a can move r steps in Direction dir
bool StudentWorld::canMoveInDirection(Actor* a, GraphObject::Direction dir, int r) const
{
	int x = a->getX();
	int y = a->getY();
	switch (dir)
	{
	case GraphObject::right:
		x++;
		for (int i = 0; i < r; i++, x++)
			if (!canMoveTo(a, x, y))
				return false;
		break;
	case GraphObject::left:
		x--;
		for (int i = 0; i < r; i++, x--)
			if (!canMoveTo(a, x, y))
				return false;
		break;
	case GraphObject::up:
		y++;
		for (int i = 0; i < r; i++, y++)
			if (!canMoveTo(a, x, y))
				return false;
		break;
	case GraphObject::down:
		y--;
		for (int i = 0; i < r; i++, y--)
			if (!canMoveTo(a, x, y))
				return false;
		break;
	}
	return true;
}

// Return whether an Actor is within radius r of the FrackMan.
bool StudentWorld::nearFrackMan(Actor* a, double r) const
{
	return withinRadius(m_player->getX(), m_player->getY(), a->getX(), a->getY(), r);
}

// Return whether an Actor is facing toward the direction of the FrackMan.
bool StudentWorld::facingTowardFrackMan(Actor* a) const
{
	return (a->getX() > m_player->getX() && a->getDirection() == GraphObject::left)
		|| (a->getX() < m_player->getX() && a->getDirection() == GraphObject::right)
		|| (a->getY() > m_player->getY() && a->getDirection() == GraphObject::down)
		|| (a->getY() < m_player->getY() && a->getDirection() == GraphObject::up);
}

// Return whether an Actor is in the direct line of sight to the FrackMan
GraphObject::Direction StudentWorld::lineOfSightToFrackMan(Actor* a) const
{
	int x = a->getX();
	int y = a->getY();
	const int x_player = m_player->getX();
	const int y_player = m_player->getY();
	//if (x == x_player && y == y_player)
	//{
	//	return a->getDirection();
	//}
	if (a->getX() == x_player)	// Search up and down
	{
		GraphObject::Direction dir = GraphObject::up;
		while (canMoveTo(a, x, y))
		{
			if (y == y_player)
				return dir;
			y++;
		}
		dir = GraphObject::down;
		y = a->getY();
		while (canMoveTo(a, x, y))
		{
			if (y == y_player)
				return dir;
			y--;
		}
	}
	y = a->getY();
	if (a->getY() == y_player)	// Search left and right
	{
		GraphObject::Direction dir = GraphObject::left;
		while (canMoveTo(a, x, y))
		{
			if (x == x_player)
				return dir;
			x--;
		}
		dir = GraphObject::right;
		x = a->getX();
		while (canMoveTo(a, x, y))
		{
			if (x == x_player)
				return dir;
			x++;
		}
	}

	return GraphObject::none;
}



void StudentWorld::setDisplayText()
{
	int score = getScore();
	int level = getLevel();
	int lives = getLives();
	int health = m_player->getHealth();
	int water = m_player->getWater();
	int gold = m_player->getGold();
	int sonar = m_player->getSonar();
	int barrels = m_nBarrels;
	string s = formatScore(score, level, lives, health, water, gold, sonar, barrels);
	setGameStatText(s);
}

string StudentWorld::formatScore(int score, int level, int lives, int health, int water, int gold, int sonar, int barrels) const
{
	string scr = formatStat(score, 6, '0');
	string lvl = formatStat(level, 2);
	string live = formatStat(lives, 1);
	string hlth = formatStat(health, 2) + "0%";
	string wtr = formatStat(water, 2);
	string gld = formatStat(gold, 2);
	string snr = formatStat(sonar, 2);
	string oil = formatStat(barrels, 2);
	return "Scr: " + scr + "  Lvl: " + lvl + "  Lives: " + live + "  Hlth: " + hlth + "  Wtr: " + wtr
		+ "  Gld: " + gld + "  Sonar: " + snr + "  Oil Left: " + oil;
}

string StudentWorld::formatStat(int stat, int digits, char leadChar) const
{
	string s = "";
	char c = ' ';
	int divisor = 1;
	bool startOf = true;
	for (int i = 1; i < digits; i++)
		divisor *= 10;
	while (divisor != 0)
	{
		int digit = stat / divisor;
		if (startOf && digit == 0 && divisor != 1)
			s += leadChar;
		else
		{
			startOf = false;
			c = '0' + digit;
			s += c;
			if (digit != 0)
				stat %= (digit * divisor);
		}
		divisor /= 10;
	}
	return s;
}

bool StudentWorld::finishedLevel() const
{
	return m_nBarrels == 0;
}

bool StudentWorld::withinRadius(int x1, int y1, int x2, int y2, double r) const
{
	return distance(x1, y1, x2, y2) <= r;
}

bool StudentWorld::dirtLess(int x, int y) const
{
	for (int i = y; i < VIEW_HEIGHT - 4 && i <= y + 3; i++)
		for (int j = x; j < VIEW_WIDTH && j <= x + 3; j++)
			if (m_dirt[i][j] != nullptr)
				return false;
	return true;
}

// Return whether the 4x4 square of x, y is fully immersed in dirt
bool StudentWorld::inDirt(int x, int y) const
{
	for (int i = y; i < VIEW_HEIGHT - 4 && i <= y + 3; i++)
		for (int j = x; j < VIEW_WIDTH && j <= x + 3; j++)
			if (m_dirt[i][j] == nullptr)
				return false;
	return true;
}

void StudentWorld::pickLocation(int& x, int& y, int lowerx, int upperx, int lowery, int uppery)
{
	x = randInt(lowerx, upperx);
	y = randInt(lowery, uppery);
}

void StudentWorld::pickValidLocation(int&x, int &y)
{
	bool validLocation;
	do
	{
		validLocation = true;
		pickLocation(x, y);
		if (!inDirt(x, y))
		{
			validLocation = false;
			continue;
		}
		for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++)
			if (distance(x, y, (*it)->getX(), (*it)->getY()) <= 6)
			{
				validLocation = false;
				break;
			}
	} while (!validLocation);
}

void StudentWorld::locationInFront(Actor* a, int r, int& x, int& y)
{
	switch (a->getDirection())
	{
	case GraphObject::right:
		x = a->getX() + r;
		y = a->getY();
		break;
	case GraphObject::left:
		x = a->getX() - r;
		y = a->getY();
		break;
	case GraphObject::up:
		y = a->getY() + r;
		x = a->getX();
		break;
	case GraphObject::down:
		y = a->getY() - r;
		x = a->getX();
		break;
	}
}

// Returns the distance between two points (x1, y1) and (x2, y2)
double StudentWorld::distance(int x1, int y1, int x2, int y2) const
{
	double dx = x1 - x2;
	double dy = y1 - y2;
	return sqrt(dx*dx + dy*dy);
}

void StudentWorld::updateMaze(int maze[][VIEW_WIDTH - 3], int ex, int ey)
{
	for (int i = 0; i < VIEW_HEIGHT - 3; i++)
	{
		for (int j = 0; j < VIEW_WIDTH - 3; j++)
		{
			if (dirtLess(j, i) && canMoveTo(m_player, j, i))
				maze[i][j] = -1;	// Valid location that is not marked yet
			else
				maze[i][j] = -2;	// Can't move to this square
		}
	}
	queue<Coord> mazequeue;
	Coord start(ey, ex);
	int stepsAway = 1;
	int lastCount = 1;
	maze[ey][ex] = 0;	// Destination is 0 steps away
	mazequeue.push(start);
	while (!mazequeue.empty())
	{
		Coord front = mazequeue.front();
		int r = front.r();	// y coordinate
		int c = front.c();	// x coordinate
		mazequeue.pop();
		lastCount--;
		if (r + 1 < VIEW_HEIGHT - 3 && maze[r + 1][c] == -1)
		{
			Coord p(r + 1, c);
			mazequeue.push(p);
			maze[r + 1][c] = stepsAway;
		}
		if (c + 1 < VIEW_WIDTH - 3 && maze[r][c + 1] == -1)
		{
			Coord p(r, c + 1);
			mazequeue.push(p);
			maze[r][c + 1] = stepsAway;
		}
		if (r - 1 >= 0 && maze[r - 1][c] == -1)
		{
			Coord p(r - 1, c);
			mazequeue.push(p);
			maze[r - 1][c] = stepsAway;
		}
		if (c - 1 >= 0 && maze[r][c - 1] == -1)
		{
			Coord p(r, c - 1);
			mazequeue.push(p);
			maze[r][c - 1] = stepsAway;
		}

		if (lastCount == 0)
		{
			stepsAway++;
			lastCount = mazequeue.size();
		}
	}
}

GraphObject::Direction StudentWorld::determineFirstMove(int maze[][VIEW_WIDTH - 3], int x, int y, int maxSteps)
{
	int curStepsAway = maze[y][x];
	if (curStepsAway > maxSteps)
		return GraphObject::none;
	if (y > 0)
	{
		int d = maze[y - 1][x];
		if (d > -1 && d < curStepsAway)
			return GraphObject::down;
	}

	if (y < VIEW_HEIGHT - 4)
	{
		int u = maze[y + 1][x];
		if (u > -1 && u < curStepsAway)
			return GraphObject::up;
	}

	if (x > 0)
	{
		int l = maze[y][x - 1];
		if (l > -1 && l < curStepsAway)
			return GraphObject::left;
	}

	if (x < VIEW_WIDTH - 4)
	{
		int r = maze[y][x + 1];
		if (r > -1 && r < curStepsAway)
			return GraphObject::right;
	}
	return GraphObject::none;
}

// Return a random int from min to max, inclusive
int randInt(int min, int max)
{
	if (max < min)
		swap(max, min);
	static random_device rd;
	static mt19937 generator(rd());
	uniform_int_distribution<> distro(min, max);
	return distro(generator);
}