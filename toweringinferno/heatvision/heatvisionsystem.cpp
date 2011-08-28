#include <algorithm>
#include <assert.h>
#include <climits>
#include <array>
#include "heatvisionsystem.h"
#include "../world.h"

namespace toweringinferno
{
namespace heatvision
{

bool isTopRow(
	const Tile t
	)
{
	return t >= eTile_TopLeft && t <= eTile_TopRight;
}

bool isBottomRow(
	const Tile t
	)
{
	return t >= eTile_BottomLeft && t <= eTile_BottomRight;
}

bool isLeft(
	const Tile t
	)
{
	return t == eTile_TopLeft || t == eTile_Left || t == eTile_BottomLeft;
}

bool isRight(
	const Tile t
	)
{
	return t == eTile_TopRight || t == eTile_Right || t == eTile_BottomRight;
}

Tile calculateTile(
	const Tile origin,
	const Tile direction
	)
{
	return (isLeft(origin) && isLeft(direction)
			|| isRight(origin) && isRight(direction)
			|| isTopRow(origin) && isTopRow(direction)
			|| isBottomRow(origin) && isBottomRow(direction)) 
		? eTile_Count
		: static_cast<Tile>(origin + (direction - eTile_Origin));
}

bool isValidCivilianCell(
	const CellType type
	)
{
	return type == eFloor || type == eOpenDoor;
}

template<typename t_CivilianConstIterator>
float calculateDanger(
	const Position& pos,
	const Tile tile,
	const World& world,
	const t_CivilianConstIterator& civiliansBegin,
	const t_CivilianConstIterator& civiliansEnd
	)
{
	const Cell& cell = world.getCell(pos);

	const t_CivilianConstIterator occupyingCivilian = tile == eTile_Origin 
		? civiliansEnd // assume there's always a civilian at the center - us
		: std::find(civiliansBegin, civiliansEnd, pos);

	return isValidCivilianCell(cell.type) == false ? 1.0f
		: cell.fire > 0.0f ? 1.0f
		: occupyingCivilian != civiliansEnd ? 1.0f
		: cell.heat;
}

float calculateDesire(
	const Position& pos,
	const Position& origin
	)
{
	return pos == origin ? 0.5f : 0.0f;
}

bool operator<(const TileHeat& lhs, const TileHeat& rhs)
{
	return lhs.danger < rhs.danger ? true
		: lhs.danger == rhs.danger ? lhs.desire > rhs.desire
		: false;
}

void gaussianBlur(
	const TileHeat* const tileHeatsIn,
	const float blurFactor,
	TileHeat* const tileHeatsOut
	)
{
	for(int tileIndex = 0; tileIndex < eTile_Count; ++tileIndex)
	{
		const Tile currentTile = static_cast<Tile>(tileIndex);
		TileHeat runningTileHeat(tileHeatsIn[tileIndex]);
		float contribution = 1.0f;

		for(int neighbourIndex = 0; neighbourIndex < eTile_Count; ++neighbourIndex)
		{
			const Tile neighbourDirection = static_cast<Tile>(neighbourIndex);
			const Tile neighbour = calculateTile(currentTile, neighbourDirection);
			
			if (neighbour == eTile_Count)
			{
				continue;
			}

			const TileHeat& neighbourTile = tileHeatsIn[neighbour];
			contribution += blurFactor;

			runningTileHeat.danger += neighbourTile.danger * blurFactor;
			runningTileHeat.desire += neighbourTile.desire * blurFactor;
		}

		runningTileHeat.danger /= contribution;
		runningTileHeat.desire /= contribution;

		tileHeatsOut[tileIndex] = runningTileHeat;
	}
}

} // namespace heatvision
} // namespace toweringinferno

toweringinferno::Position toweringinferno::heatvision::calculatePosition(
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

toweringinferno::heatvision::HeatvisionSystem::HeatvisionSystem()
{
}

void toweringinferno::heatvision::HeatvisionSystem::preUpdate()
{
	for(auto civilianIt = m_civilians.begin(); civilianIt != m_civilians.end(); ++civilianIt)
	{
		civilianIt->pos = civilianIt->nextPos;
	}
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

			const float danger = calculateDanger(tilePos, currentTile, world, m_civilians.begin(), m_civilians.end());
			const float desire = calculateDesire(tilePos, origin);

			heat[tileIndex] = TileHeat(tilePos, danger, desire);
		}

		gaussianBlur(heat, 0.2f, civilianIt->heatMap);

		std::sort(civilianIt->heatMap, civilianIt->heatMap + eTile_Count);

		civilianIt->nextPos = civilianIt->heatMap[0].pos;
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
