#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <math.h>
#include <array>

using namespace std;

float dist(int x1, int y1, int x2, int y2)
{
	return sqrt((float)((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
}

enum StructureType {NONE = -1, BARRACKS = 2};
enum UnitType { QUEEN = -1, KNIGHT = 0, ARCHER = 1 };

class Player;
class Site;

class Structure
{
public:
	Player* owner;
	Site* site;

};

class Barracks : public Structure
{
public:
	UnitType unitType;
	int timeUntilIdle;
	int trainCost;
};

class Site
{
public:
	int ID;
	int row;
	int col;
	int radius;
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


class Unit
{
public:
	int row;
	int col;
	Player* owner;
	UnitType type;
	int health;

};

class Board
{
public:
	map<int, Site> sites;

	const Site* closestUnusedSite(Unit& queen) const
	{
		float minDist = 99999.0f;
		const Site* answer = nullptr;

		for (auto& siteIter : sites)
		{
			const Site& site = siteIter.second;
			if (site.structure != nullptr) { continue; }
			float tempDist = dist(queen.col, queen.row, site.col, site.row);
			if (tempDist < minDist)
			{
				minDist = tempDist;
				answer = &site;
			}
		}

		return answer;
	}

};

class Player
{
public:
	int gold;
	int touchedSiteID;
	vector<Barracks> ownedBarracks;
	Unit queen;
	vector<Unit> units;

	array<int, 2> getUnitCentroid() const
	{
		if (units.empty()) { return { {-1, -1} }; }
		int centroidRow = 0;
		int centroidCol = 0;
		for (auto& unit : units)
		{
			centroidRow += unit.row;
			centroidCol += unit.col;
		}

		centroidRow /= units.size();
		centroidCol/= units.size();

		return { {centroidRow, centroidCol} };

	}
};



int main()
{
	Board board;
	Player hero, enemy;
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
		hero.units.clear();
		enemy.units.clear();

		hero.gold = gold;
		hero.touchedSiteID = touchedSite;
        for (int i = 0; i < numSites; i++) {
            int siteId;
            int ignore1; // used in future leagues
            int ignore2; // used in future leagues
            int structureType; // -1 = No structure, 2 = Barracks
            int owner; // -1 = No structure, 0 = Friendly, 1 = Enemy
            int param1;
            int param2;
            cin >> siteId >> ignore1 >> ignore2 >> structureType >> owner >> param1 >> param2; cin.ignore();

			Site& site = board.sites[siteId];
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
				site.structure = &player.ownedBarracks.front();
			}

        }
        int numUnits;
        cin >> numUnits; cin.ignore();
        for (int i = 0; i < numUnits; i++) {
            int x;
            int y;
            int owner;
            int unitType; // -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER
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

			else
			{
				Unit tempUnit;
				tempUnit.row = y;
				tempUnit.col = x;
				tempUnit.health = health;
				tempUnit.owner = &player;
				tempUnit.type = unitType == 0 ? KNIGHT : ARCHER;
			}

        }

		int myBarracksOwned = hero.ownedBarracks.size();

		if (myBarracksOwned < 2)
		{
			const Site* closesetUnusedSite = board.closestUnusedSite(hero.queen);
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
				buildKnights = !buildKnights;
			}
		}
		cout << endl;
    }
}