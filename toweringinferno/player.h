#ifndef __TI_PLAYER_H_
#define __TI_PLAYER_H_

#include "position.h"
#include "libtcod.hpp"

namespace toweringinferno
{

class World;

class Player
{
public:
	Player();

	void resetForNewFloor();

	void update(World& world);

	const Position& getPos() const { return m_pos; }
	void setPos(const Position& pos) { setPos(pos.first, pos.second); }
	void setPos(int x, int y) { m_pos = Position(x, y); }
	float getHealth()const { return m_levelData.health; }
	bool isDead() const { return m_levelData.health == 0.0f; }

	void useWaterBomb(World& world);

	int getBombsRemaining() const { return m_levelData.waterBombs; }
	int getCiviliansRescued() const { return m_levelData.civiliansRescued; }
	int getScore() const { return m_score; }

private:

	struct FloorSpecificData
	{
		FloorSpecificData();
		float health;
		int waterBombs;
		int civiliansRescued;
	};

	Position m_pos;
	FloorSpecificData m_levelData;
	int m_score;
}; 

} // namespace toweringinferno

#endif // __TI_PLAYER_H_