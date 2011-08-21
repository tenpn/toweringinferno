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

} // namespace toweringinferno

toweringinferno::World::World(
	const int w, 
	const int h
	)
	: m_map(w*h)
	, m_width(w)
	, m_height(h)
{

}

toweringinferno::Position toweringinferno::World::calculateNewPlayerPos(
	const TCOD_keycode_t movementDir, 
	const Position& playerPos
	)const
{
	const Position idealNewPosition = calculateIdealNewPlayerPosition(playerPos, movementDir);
	const CellType newPositionType = getType(idealNewPosition);
	return isValidPlayerCell(newPositionType) ? idealNewPosition : playerPos;
}

toweringinferno::WorldEvents toweringinferno::World::update(
	const TCOD_keycode_t movementDir
	)
{
	if (m_player.isDead())
	{
		return eEvent_PlayerDied;
	}

	m_player.setPos(calculateNewPlayerPos(movementDir, m_player.getPos()));
	m_player.update(*this);

	updateDynamics();

	return getType(m_player.getPos()) == eStairsDown ? eEvent_NextFloorDown 
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
				
			const float evaporation = utils::mapValue(cell.heat, 0.0f, 1.0f, 0.1f, 0.6f);
			float waterTotal = cell.water - evaporation;
			int waterContributors = 1;

			float condensationScore = cell.water;
			int condensationContributors = 1;

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

					condensationScore += neighbour.water;
					condensationContributors += neighbour.water > 0.0f ? 1 : 0;

					const bool isNeighbourContributingWater = neighbour.type != eWall && neighbour.type != eSky
						&& cell.water < neighbour.water;
					waterTotal += (isNeighbourContributingWater ? neighbour.water : 0.0f);
					waterContributors += isNeighbourContributingWater ? 1 : 0;
				}
			}

			cell.waterFlip = cell.type == eWall || cell.type == eSky ? 0.0f
				: utils::max(waterTotal / static_cast<float>(waterContributors), 0.0f);
			
			const float heatBuildRate = cell.type == eWall ? 0.6f : 0.2f;

			const float condensation = utils::mapValue(condensationScore / static_cast<float>(condensationContributors), 
				0.0f, 1.0f, 0.01f, 0.5f);
			cell.heatFlip = utils::clamp( cell.heat + heat * heatBuildRate - condensation, 0.0f, 1.0f);

			if (cell.type == eWall)
			{
				cell.hp = utils::max(0.0f, cell.hp - utils::mapValue(cell.fire, 0.9f, 1.0f, 0.0f, 0.05f));

				if (cell.hp == 0.0f)
				{
					cell.type = eFloor;
					cell.fire = 0.75f;
				}
			}
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
			else
			{
				cell.fire = 0.0f;
			}
		}
	}
}
