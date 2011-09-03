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
	Cell() : type(eSky), fire(0.0f), heat(0.0f), water(0.0f), waterFlip(0.0f), heatFlip(0.0f), typeFlip(eSky), hp(1.0f) {}

	CellType type;
	float fire;
	float heat;
	float water;
	float waterFlip;
	float heatFlip;
	CellType typeFlip;
	float hp;

	void setFire(const float fireIn) { fire = heat = fireIn; }
};

class World
{
public:
	World(int w, int h);

	void resetForNewFloor();

	WorldEvents update(const TCOD_key_t& command);

	CellType getType(const Point& pos)const { return getType(pos.col, pos.row); }
	CellType getType(int x, int y)const;
	const Cell& getCell(const Point& pos)const { return getCell(pos.col, pos.row); }
	const Cell& getCell(int x, int y)const;
	void set(int x, int y, CellType newType);
	void setFire(int x, int y, float newFire);
	void setWaterBomb(const Point& pos);
	void setHose(int x, int y);

	bool isValidCoords(const int x, const int y)const { return x >= 0 && x < m_width && y >= 0 && y < m_height; }
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

	const Player& getPlayer() const { return m_player; }
	Player& getPlayer() { return m_player; }

	bool isSprinklerSystemAvailable() const { return m_floorData.isSprinklerAvailable; }

	int getFloorsEscaped() const { return m_floorsEscaped; }

private:
	
	enum ActionSuccess
	{
		eAction_Succeeded,
		eAction_InvalidInput,
		eAction_Failed,
	};

	ActionSuccess updateDoors(const TCOD_key_t& command);
	ActionSuccess updateSprinklerControl(const TCOD_key_t& command);
	ActionSuccess updateHoseRelease(const TCOD_key_t& command);
	ActionSuccess updateAxe(const TCOD_key_t& command);
	void updateDynamics();
	ActionSuccess calculateNewPlayerPos(TCOD_keycode_t movementDir, const Point& playerPos);

	inline int coordsToIndex(const Point& pos) const { return coordsToIndex(pos.col, pos.row); }
	int coordsToIndex(int x, int y) const;

	struct FloorSpecificData
	{
		FloorSpecificData(int w, int h);

		std::vector<Cell> map;
		bool isSprinklerAvailable;
		TCOD_keycode_t lastMovementDir;
	};

	int m_width;
	int m_height;
	Player m_player;
	FloorSpecificData m_floorData;
	int m_floorsEscaped;
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
	assert(index < static_cast<int>(m_floorData.map.size()));
	return index;
}

inline
const toweringinferno::Cell& toweringinferno::World::getCell(
	const int x,
	const int y
	) const
{
	return m_floorData.map[coordsToIndex(x,y)];
}

inline
void toweringinferno::World::set(
	const int x, 
	const int y,
	const CellType newType
	)
{
	m_floorData.map[coordsToIndex(x,y)].type = newType;
}

inline
void toweringinferno::World::setWaterBomb(
	const Point& pos
	)
{
	m_floorData.map[coordsToIndex(pos.col,pos.row)].water = 1.5f;
}

inline
void toweringinferno::World::setHose(
	const int x, 
	const int y
	)
{
	Cell& cell = m_floorData.map[coordsToIndex(x,y)];
	cell.type = eHose;
	cell.water = 0.0f;
}

inline
void toweringinferno::World::setFire(
	const int x, 
	const int y, 
	const float newFire
	)
{
	Cell& cell = m_floorData.map[coordsToIndex(x,y)];
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
		: m_floorData.map[coordsToIndex(x,y)].type;
}

#endif // __TI_WORLD_H_