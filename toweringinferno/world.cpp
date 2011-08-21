#include <array>
#include "world.h"
#include "utils.h"


namespace toweringinferno
{

Position calculateIdealNewPlayerPosition(
	const Position& current,
	const TCOD_keycode_t movementDir
	)
{
	switch(movementDir)
	{
	case TCODK_LEFT:
		return Position(current.first - 1, current.second);

	case TCODK_RIGHT:
		return Position(current.first + 1, current.second);

	case TCODK_UP:
		return Position(current.first, current.second - 1);

	case TCODK_DOWN:
		return Position(current.first, current.second + 1);

	default:
		return current;
	}
}

bool isValidPlayerCell(
	const CellType cell
	)
{
	return cell == eFloor || cell == eStairsDown || cell == eStairsUp;
}

float calculateDamage(
	const Cell& cell
	)
{
	return cell.fire > 0.2f 
		? 0.25f
		: utils::mapValue(cell.heat, 0.0f, 1.0f, 0.0f, 0.1f);
}

} // namespace toweringinferno

toweringinferno::World::World(
	const int w, 
	const int h
	)
	: m_map(w*h)
	, m_width(w)
	, m_height(h)
	, m_playerPos(static_cast<int>(w*0.25f), static_cast<int>(h*0.25f))
	, m_playerHealth(1.0f)
{

}

toweringinferno::Position toweringinferno::World::calculateNewPlayerPos(
	const TCOD_keycode_t movementDir
	)const
{
	const Position idealNewPosition = calculateIdealNewPlayerPosition(m_playerPos, movementDir);
	const CellType newPositionType = getType(idealNewPosition);
	return isValidPlayerCell(newPositionType) ? idealNewPosition : m_playerPos;
}

toweringinferno::WorldEvents toweringinferno::World::update(
	const TCOD_keycode_t movementDir
	)
{
	if (m_playerHealth == 0.0f)
	{
		return eEvent_PlayerDied;
	}

	m_playerPos = calculateNewPlayerPos(movementDir);
	m_playerHealth = utils::max(m_playerHealth - calculateDamage(getCell(m_playerPos)), 0.0f);

	updateDynamics();

	return getType(m_playerPos) == eStairsDown ? eEvent_NextFloorDown 
		: eEvent_None;
}

void toweringinferno::World::updateDynamics()
{
	for(int col = 0; col < getWidth(); ++col)
	{
		for(int row = 0; row < getHeight(); ++row)
		{
			Cell& cell = m_map[coordsToIndex(col, row)];

			if (cell.type == eSky)
			{
				continue;
			}

			// calculate heat
			float heat = 0.0f;
				
			float waterTotal = cell.water - 0.1f;
			int waterContributors = 1;

 			for(int neighbourCol = utils::max(col - 1, 0); 
				neighbourCol < utils::min(getWidth(), col + 2);
				++neighbourCol)
			{
				for (int neighbourRow = utils::max(row - 1, 0);
					neighbourRow < utils::min(getHeight(), row + 2);
					++neighbourRow)
				{
					if (neighbourCol == col && neighbourRow == row)
					{
						continue;
					}

					Cell& neighbour = m_map[coordsToIndex(neighbourCol,neighbourRow)];

					const bool wallCanContributeHeat 
						= (row == neighbourRow && (getType(col, row - 1) == eFloor || getType(col, row + 1) == eFloor))
						|| (col == neighbourCol && (getType(col - 1, row) == eFloor || getType(col + 1, row) == eFloor));

					const float heatContribution 
						= neighbour.type == eWall && cell.type == eWall && wallCanContributeHeat == false ? 0.0f
						: neighbour.type == eWall ? (3.0f/12.0f)
						: (1.0f/12.0f);
					
					heat += neighbour.heat * heatContribution;

					const bool isNeighbourContributingWater = neighbour.type != eWall && neighbour.type != eSky
						&& cell.water < neighbour.water;
					waterTotal += (isNeighbourContributingWater ? neighbour.water : 0.0f);
					waterContributors += isNeighbourContributingWater ? 1 : 0;
				}
			}

			cell.waterFlip = cell.type == eWall || cell.type == eSky ? 0.0f
				: utils::max(waterTotal / static_cast<float>(waterContributors), 0.0f);
			
			const float heatBuildRate = cell.type == eWall ? 0.6f : 0.2f;

			cell.heatFlip = utils::min( cell.heat + heat * heatBuildRate, 1.0f);
		}
	}

	for(int col = 0; col < getWidth(); ++col)
	{
		for(int row = 0; row < getHeight(); ++row)
		{
			Cell& cell = m_map[coordsToIndex(col, row)];
			cell.water = cell.waterFlip;

			cell.heat = cell.heatFlip;

			const float fireThreshold 
				= cell.type == eWall ? 0.5f
				: 0.9f;

			if (cell.heat > fireThreshold)
			{
				cell.fire = utils::clamp(cell.fire + utils::mapValue(cell.heat, fireThreshold, 1.0f, 0.0f, 0.05f), 0.0f, 1.0f);
			}
		}
	}
}
