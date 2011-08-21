#ifndef __TI_WORLD_H_
#define __TI_WORLD_H_

#include <assert.h>
#include <vector>
#include "libtcod.hpp"
#include "celltype.h"
#include "position.h"
#include "player.h"

namespace toweringinferno
{

enum WorldEvents
{
	eEvent_InvalidInput,
	eEvent_None,
	eEvent_NextFloorDown,
	eEvent_PlayerDied,
};

struct Cell
{
	Cell() : type(eSky), fire(0.0f), heat(0.0f), water(0.0f), waterFlip(0.0f), heatFlip(0.0f), hp(1.0f) {}

	CellType type;
	float fire;
	float heat;
	float water;
	float waterFlip;
	float heatFlip;
	float hp;

	void setFire(const float fireIn) { fire = heat = fireIn; }
};

class World
{
public:
	World(int w, int h);

	WorldEvents update(TCOD_key_t command);

	CellType getType(const Position& pos)const { return getType(pos.first, pos.second); }
	CellType getType(int x, int y)const;
	const Cell& getCell(const Position& pos)const { return getCell(pos.first, pos.second); }
	const Cell& getCell(int x, int y)const;
	void set(int x, int y, CellType newType);
	void setFire(int x, int y, float newFire);
	void setWaterBomb(const Position& pos);
	void setHose(int x, int y);

	bool isValidCoords(const int x, const int y)const { return x >= 0 && x < m_width && y >= 0 && y < m_height; }
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

	const Player& getPlayer() const { return m_player; }
	Player& getPlayer() { return m_player; }

private:
	
	enum DoorActionSuccess
	{
		eDoorAction_FlippedDoor,
		eDoorAction_InvalidInput,
		eDoorAction_NoDoorsFlipped,
	};

	DoorActionSuccess updateDoors(TCOD_key_t command);
	void updateDynamics();
	Position calculateNewPlayerPos(TCOD_keycode_t movementDir, const Position& playerPos)const;

	inline int coordsToIndex(const Position& pos) const { return coordsToIndex(pos.first, pos.second); }
	int coordsToIndex(int x, int y) const;

	std::vector<Cell> m_map;
	int m_width;
	int m_height;
	Player m_player;
}; 

} // namespace toweringinferno

inline
int toweringinferno::World::coordsToIndex(
	const int x, 
	const int y
	) const
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);
	const int index = y * m_width + x;
	assert(index < static_cast<int>(m_map.size()));
	return index;
}

inline
const toweringinferno::Cell& toweringinferno::World::getCell(
	const int x,
	const int y
	) const
{
	return m_map[coordsToIndex(x,y)];
}

inline
void toweringinferno::World::set(
	const int x, 
	const int y,
	const CellType newType
	)
{
	m_map[coordsToIndex(x,y)].type = newType;
}

inline
void toweringinferno::World::setWaterBomb(
	const Position& pos
	)
{
	m_map[coordsToIndex(pos.first,pos.second)].water = 1.5f;
}

inline
void toweringinferno::World::setHose(
	const int x, 
	const int y
	)
{
	Cell& cell = m_map[coordsToIndex(x,y)];
	cell.type = eHose;
	cell.water = 3.0f;
}

inline
void toweringinferno::World::setFire(
	const int x, 
	const int y, 
	const float newFire
	)
{
	Cell& cell = m_map[coordsToIndex(x,y)];
	cell.fire = newFire;
	cell.heat = 1.0f;
}

inline
toweringinferno::CellType toweringinferno::World::getType(
	const int x, 
	const int y
	)const
{
	return x < 0 || x >= m_width ? eSky
		: y < 0 || y >= m_height ? eSky
		: m_map[coordsToIndex(x,y)].type;
}

#endif // __TI_WORLD_H_