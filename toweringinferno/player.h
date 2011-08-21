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

	void update(World& world);

	const Position& getPos() const { return m_pos; }
	void setPos(const Position& pos) { setPos(pos.first, pos.second); }
	void setPos(int x, int y) { m_pos = Position(x, y); }
	float getHealth()const { return m_health; }
	bool isDead() const { return m_health == 0.0f; }

	void useWaterBomb(World& world);

	int getBombsRemaining() const { return m_waterBombs; }
	int getCiviliansRescued() const { return m_civiliansRescued; }

private:
	Position m_pos;
	float m_health;

	int m_waterBombs;
	int m_civiliansRescued;
}; 

} // namespace toweringinferno

#endif // __TI_PLAYER_H_