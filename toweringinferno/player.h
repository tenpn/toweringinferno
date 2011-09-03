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

	void resetForNewFloor(int floorsCleared);

	void update(World& world);

	const Point& getPos() const { return m_pos; }
	void setPos(const Point& pos) { setPos(pos.col, pos.row); }
	void setPos(int x, int y) { m_pos = Point(x, y); }
	float getHealth()const { return m_health; }
	bool isDead() const { return m_health == 0.0f; }

	void useWaterBomb(World& world);
	void useAxe() { --m_axeCount; }
	void rescueCivilian() { ++m_levelData.civiliansRescued; }

	int getAxesRemaining() const { return m_axeCount; }
	int getBombsRemaining() const { return m_waterBombs; }
	int getCiviliansRescued() const { return m_levelData.civiliansRescued; }
	int getScore() const { return m_score; }

private:

	struct FloorSpecificData
	{
		FloorSpecificData();
		int civiliansRescued;
	};

	Point m_pos;
	FloorSpecificData m_levelData;
	int m_score;
	int m_waterBombs;
	float m_health;
	int m_axeCount;
}; 

} // namespace toweringinferno

#endif // __TI_PLAYER_H_