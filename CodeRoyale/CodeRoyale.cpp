#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <math.h>
#include <array>
#include <queue>

using namespace std;

struct AstarPos
{
	array<int, 2> pos;
	int fScore;
};

bool operator<(const AstarPos& lhs, const AstarPos& rhs)
{
	return lhs.fScore < rhs.fScore;
}

float dist(int x1, int y1, int x2, int y2)
{
	return sqrt((float)((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
}

float magOfVector(array<int, 2> & vect)
{
	return sqrt((float)(vect[0] * vect[0] + vect[1] * vect[1]));
}

array<int, 2> rotateDegrees(array<int, 2> initialVect, int degrees)
{
	array<int, 2> answer;
	float radians = degrees * 0.0174533f;

	answer[0] = initialVect[0] * cosf(radians) - initialVect[1] * sinf(radians);
	answer[1] = initialVect[0] * sinf(radians) + initialVect[1] * cosf(radians);

	return answer;
}

enum StructureType { NONE = -1, MINE = 0, TOWER = 1, BARRACKS = 2 };
enum UnitType { QUEEN = -1, KNIGHT = 0, ARCHER = 1, GIANT = 2 };

class Player;
class Site;

class Structure
{
public:
	Player * owner;
	Site* site;

};

class Barracks : public Structure
{
public:
	UnitType unitType;
	int timeUntilIdle;
	int trainCost;
};

class Mine : public Structure
{
public:
	int incomeRate;

};

class Tower : public Structure
{
public:
	int health;
	int radius;
};

class Site
{
public:
	int ID;
	int row;
	int col;
	int radius;
	int goldRemaining;
	int maxGoldRate;
	Structure* structure;

	Site(int ID, int row, int col, int radius)
		: ID(ID)
		, row(row)
		, col(col)
		, radius(radius)
		, structure(nullptr)
	{}

	Site()
		: ID(0)
		, row(0)
		, col(0)
		, radius(0)
		, structure(nullptr)
	{}

};

class Board;

class Unit
{
public:
	int row;
	int col;
	Player* owner;
	UnitType type;
	int health;

};

class Player
{
public:
	int gold;
	int incomeRate;
	int touchedSiteID;
	vector<Barracks> ownedBarracks;
	vector<Tower> ownedTowers;
	vector<Mine> ownedMines;
	Unit queen;
	vector<Unit> knights;
	vector<Unit> archers;
	vector<Unit> giants;

	void setIncomeRate()
	{
		incomeRate = 0;

		for (auto& mine : ownedMines)
		{
			incomeRate += mine.incomeRate;
		}
	}

	Mine* getUnMaxedMine()
	{
		for (auto& mine : ownedMines)
		{
			if (mine.incomeRate < mine.site->maxGoldRate)
			{
				return &mine;
			}
		}

		return nullptr;
	}

	bool knightsCloseToQueen(Unit& queen)
	{
		for (auto& knight : knights)
		{
			if (dist(knight.row, knight.col, queen.row, queen.col) < 200.0f)
			{
				return true;
			}
		}

		return false;
	}

};

class Board
{
public:
	map<int, Site> sites;
	Player& hero, enemy;

	Board(Player& hero, Player& enemy)
		: hero(hero)
		, enemy(enemy)
	{}

	const Site* closestUnusedSite(Unit& queen, bool minable = false) const
	{
		float minDist = 99999.0f;
		const Site* answer = nullptr;

		for (auto& siteIter : sites)
		{
			const Site& site = siteIter.second;
			if (site.structure != nullptr || (minable && site.goldRemaining == 0)) { continue; }
			float tempDist = dist(queen.col, queen.row, site.col, site.row);
			if (tempDist < minDist)
			{
				minDist = tempDist;
				answer = &site;
			}
		}

		return answer;
	}

	array<int, 2> moveQueenToward(int targetRow, int targetCol, bool isHero)
	{
		// Use A* to find the best path to take
		Unit* queen;
		if (isHero) { queen = &hero.queen; }
		else { queen = &enemy.queen; }

		priority_queue<AstarPos> openSet;
		AstarPos queenPos;
		queenPos.pos = { {queen->row, queen->col} };
		queenPos.fScore = (int)(ceil(dist(queenPos.pos[0], queenPos.pos[1], targetRow, targetCol) / 60.0f));
		openSet.push(queenPos);

		array<int, 2> best = queenPos.pos;

		while (!openSet.empty())
		{
			const AstarPos& current = openSet.top();
			if (current.pos[0] == targetRow && current.pos[1] == targetCol) { return best; }

			openSet.pop();

			// Need to cycle through each possible continuation
			// These will be defined according to angles towards destination
			// For now, try 27, 54, 81 degrees to both sides, as well as 0
			// Account for site collisions to determine where I'd end up
			array<array<int, 2>, 7> neighbors;

			// First 0 degrees
			array<int, 2> vectTowardsTarget = { {targetRow - current.pos[0], targetCol - current.pos[1]} };
			float distToTarget = magOfVector(vectTowardsTarget);
			vectTowardsTarget[0] *= 60.0f / distToTarget;
			vectTowardsTarget[1] *= 60.0f / distToTarget;
			float insideDist = distInsideSite(current.pos[0] + vectTowardsTarget[0], current.pos[1] + vectTowardsTarget[1]);
			vectTowardsTarget[0] *= (60.0f - insideDist) / 60.0f;
			vectTowardsTarget[1] *= (60.0f - insideDist) / 60.0f;
			neighbors[0][0] = current.pos[0] + vectTowardsTarget[0];
			neighbors[0][1] = current.pos[1] + vectTowardsTarget[1];

			//Then rotate and try again
			vectTowardsTarget = rotateDegrees(vectTowardsTarget, 27);
			// Continue from here
		}


	}

	float distInsideSite(int row, int col)
	{		
		//Queen radius is 30
		
		for (auto& siteIter : sites)
		{
			Site& site = siteIter.second;
			float insideDist = site.radius + 30 - dist(row, col, site.row, site.col);
			if (insideDist > 0)
			{
				return insideDist;
			}
		}

		return 0.0f;
	}

};






int main()
{
	Player hero, enemy;
	Board board = Board(hero, enemy);
	enemy.gold = -1;
	int numSites;
	cin >> numSites; cin.ignore();
	for (int i = 0; i < numSites; i++) {
		int siteId;
		int x;
		int y;
		int radius;
		cin >> siteId >> x >> y >> radius; cin.ignore();
		board.sites.emplace(piecewise_construct, forward_as_tuple(siteId), forward_as_tuple(siteId, y, x, radius));
	}

	bool buildKnights = true;

	// game loop
	while (1) {
		int gold;
		int touchedSite; // -1 if none
		cin >> gold >> touchedSite; cin.ignore();
		hero.ownedBarracks.clear();
		enemy.ownedBarracks.clear();
		hero.ownedMines.clear();
		enemy.ownedMines.clear();
		hero.ownedTowers.clear();
		enemy.ownedTowers.clear();
		hero.knights.clear();
		hero.archers.clear();
		hero.giants.clear();
		enemy.knights.clear();
		enemy.archers.clear();
		enemy.giants.clear();

		hero.gold = gold;
		hero.touchedSiteID = touchedSite;
		for (int i = 0; i < numSites; i++) {
			int siteId;
			int goldRemaining;
			int maxGoldRate;
			int structureType;
			int owner;
			int param1;
			int param2;
			cin >> siteId >> goldRemaining >> maxGoldRate >> structureType >> owner >> param1 >> param2; cin.ignore();

			Site& site = board.sites[siteId];
			site.structure = nullptr;
			site.goldRemaining = goldRemaining;
			site.maxGoldRate = maxGoldRate;
			Player& player = owner == 0 ? hero : enemy;

			if (structureType == BARRACKS)
			{
				Barracks tempBarracks = Barracks();
				tempBarracks.owner = &player;
				tempBarracks.site = &site;
				tempBarracks.timeUntilIdle = param1;
				tempBarracks.unitType = param2 == 0 ? KNIGHT : ARCHER;
				tempBarracks.trainCost = param2 == 0 ? 80 : 100;
				player.ownedBarracks.push_back(tempBarracks);
				site.structure = &player.ownedBarracks.back();
			}

			else if (structureType == TOWER)
			{
				Tower tempTower = Tower();
				tempTower.owner = &player;
				tempTower.site = &site;
				tempTower.health = param1;
				tempTower.radius = param2;
				player.ownedTowers.push_back(tempTower);
				site.structure = &player.ownedTowers.back();
			}

			else if (structureType == MINE)
			{
				Mine tempMine = Mine();
				tempMine.owner = &player;
				tempMine.site = &site;
				tempMine.incomeRate = param1;
				player.ownedMines.push_back(tempMine);
				site.structure = &player.ownedMines.back();
			}

		}
		int numUnits;
		cin >> numUnits; cin.ignore();
		for (int i = 0; i < numUnits; i++) {
			int x;
			int y;
			int owner;
			int unitType; // -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER, 2 = GIANT
			int health;
			cin >> x >> y >> owner >> unitType >> health; cin.ignore();

			Player& player = owner == 0 ? hero : enemy;

			if (unitType == QUEEN)
			{
				player.queen.row = y;
				player.queen.col = x;
				player.queen.health = health;
				player.queen.owner = &player;
				player.queen.type = QUEEN;
			}

			else if (unitType == KNIGHT)
			{
				Unit tempUnit;
				tempUnit.row = y;
				tempUnit.col = x;
				tempUnit.health = health;
				tempUnit.owner = &player;
				tempUnit.type = KNIGHT;
				player.knights.push_back(tempUnit);
			}

			else if (unitType == ARCHER)
			{
				Unit tempUnit;
				tempUnit.row = y;
				tempUnit.col = x;
				tempUnit.health = health;
				tempUnit.owner = &player;
				tempUnit.type = ARCHER;
				player.archers.push_back(tempUnit);
			}

			else if (unitType == GIANT)
			{
				Unit tempUnit;
				tempUnit.row = y;
				tempUnit.col = x;
				tempUnit.health = health;
				tempUnit.owner = &player;
				tempUnit.type = GIANT;
				player.giants.push_back(tempUnit);
			}

		}

		hero.setIncomeRate();
		enemy.setIncomeRate();

		// FINISHED PARSING


		int myBarracksOwned = hero.ownedBarracks.size();
		const Site* closesetUnusedSite = board.closestUnusedSite(hero.queen);

		if (myBarracksOwned < 1)
		{
			if (closesetUnusedSite != nullptr)
			{
				if (myBarracksOwned == 0)
				{
					cout << "BUILD " << closesetUnusedSite->ID << " BARRACKS-KNIGHT" << endl;
				}
				else
				{
					cout << "BUILD " << closesetUnusedSite->ID << " BARRACKS-ARCHER" << endl;
				}
			}
		}

		else if (closesetUnusedSite != nullptr && hero.ownedTowers.empty())
		{
			cout << "BUILD " << closesetUnusedSite->ID << " TOWER" << endl;
		}

		else if (!hero.ownedTowers.empty() && enemy.knightsCloseToQueen(hero.queen))
		{
			cout << "BUILD " << hero.ownedTowers.front().site->ID << " TOWER" << endl;
		}

		// Get a minimum income rate

		else if (hero.getUnMaxedMine() != nullptr)
		{
			cout << "BUILD " << hero.getUnMaxedMine()->site->ID << " MINE" << endl;
		}

		else if (board.closestUnusedSite(hero.queen, true) != nullptr && hero.incomeRate < 8)
		{

			cout << "BUILD " << board.closestUnusedSite(hero.queen, true)->ID << " MINE" << endl;
		}


		else
		{
			cout << "WAIT" << endl;
		}



		// Train when possible
		cout << "TRAIN";
		for (auto& barracks : hero.ownedBarracks)
		{
			if (barracks.timeUntilIdle == 0 && hero.gold >= barracks.trainCost && (barracks.unitType == KNIGHT) == buildKnights)
			{
				cout << " " << barracks.site->ID;
				hero.gold -= barracks.trainCost;
				//buildKnights = !buildKnights;
			}
		}
		cout << endl;
	}
}