#include <algorithm>
#include <assert.h>
#include <climits>
#include "heatvisionsystem.h"
#include "../world.h"

namespace toweringinferno
{
namespace heatvision
{

bool operator==(
	const Civilian& civvie, 
	const Position& pos
	)
{
	return civvie.pos == pos;
}

enum Tile
{
	eTile_TopLeft,
	eTile_Top,
	eTile_TopRight,
	eTile_Left,
	eTile_Origin,
	eTile_Right,
	eTile_BottomLeft,
	eTile_Bottom,
	eTile_BottomRight,
	eTile_Count,
};

Position calculatePosition(
	const Position& origin,
	const Tile tile
	)
{
	switch(tile)
	{
	case eTile_TopLeft: return Position(origin.first-1, origin.second-1);
	case eTile_Top: return Position(origin.first, origin.second-1);
	case eTile_TopRight: return Position(origin.first+1, origin.second-1);
	case eTile_Left: return Position(origin.first-1, origin.second);
	case eTile_Origin: return origin;
	case eTile_Right: return Position(origin.first+1, origin.second);
	case eTile_BottomLeft: return Position(origin.first-1, origin.second+1);
	case eTile_Bottom: return Position(origin.first, origin.second+1);
	case eTile_BottomRight: return Position(origin.first+1, origin.second+1);

	default:
	case eTile_Count: 
		assert(false); // not expected
		return origin;
	}
}

bool isValidCivilianCell(
	const CellType type
	)
{
	return type == eFloor || type == eOpenDoor;
}

float calculateDanger(
	const Position& pos,
	const World& world
	)
{
	const Cell& cell = world.getCell(pos);
	return isValidCivilianCell(cell.type) == false ? 1.0f
		: cell.fire > 0.0f ? 1.0f
		: cell.heat;
}

float calculateDesire(
	const Position& pos,
	const Position& origin
	)
{
	return pos == origin ? 0.5f : 0.0f;
}

struct TileHeat
{
	TileHeat() : pos(INT_MAX,INT_MAX), danger(0.0f), desire(0.0f) {}
	TileHeat(const Position posIn, const float dangerIn, const float desireIn)
		: pos(posIn), danger(dangerIn), desire(desireIn)
	{}
	Position pos;
	float danger;
	float desire;
};

bool operator<(const TileHeat& lhs, const TileHeat& rhs)
{
	return lhs.danger < rhs.danger ? true
		: lhs.danger == rhs.danger ? lhs.desire > rhs.desire
		: false;
}

} // namespace heatvision
} // namespace toweringinferno

toweringinferno::heatvision::HeatvisionSystem::HeatvisionSystem()
{
}

void toweringinferno::heatvision::HeatvisionSystem::update(
	const toweringinferno::World& world
	)
{
	for(auto civilianIt = m_civilians.begin(); civilianIt != m_civilians.end(); ++civilianIt)
	{
		TileHeat heat[eTile_Count];

		const Position& origin = civilianIt->pos;

		for(int tileIndex = 0; tileIndex < eTile_Count; ++tileIndex)
		{
			const Tile currentTile = static_cast<Tile>(tileIndex);
			const Position tilePos = calculatePosition(origin, currentTile);

			const float danger = calculateDanger(tilePos, world);
			const float desire = calculateDesire(tilePos, origin);

			heat[tileIndex] = TileHeat(tilePos, danger, desire);
		}

		std::sort(heat, heat + eTile_Count);

		civilianIt->pos = heat[0].pos;
	}
}

bool toweringinferno::heatvision::HeatvisionSystem::tryRemoveCivilian(
	const Position& pos
	)
{
	const auto civilian = std::find(m_civilians.begin(), m_civilians.end(), pos);
	if (civilian != m_civilians.end())
	{
		m_civilians.erase(civilian);
		return true;
	}
	return false;
}
