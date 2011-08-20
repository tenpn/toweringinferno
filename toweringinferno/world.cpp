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

} // namespace toweringinferno

toweringinferno::World::World(
	const int w, 
	const int h
	)
	: m_map(w*h)
	, m_width(w)
	, m_height(h)
	, m_playerPos(static_cast<int>(w*0.25f), static_cast<int>(h*0.25f))
{

}

toweringinferno::Position toweringinferno::World::calculateNewPlayerPos(
	const TCOD_keycode_t movementDir
	)const
{
	const Position idealNewPosition = calculateIdealNewPlayerPosition(m_playerPos, movementDir);
	return getType(idealNewPosition) == eFloor ? idealNewPosition : m_playerPos;
}

void toweringinferno::World::update(
	const TCOD_keycode_t movementDir
	)
{
	m_playerPos = calculateNewPlayerPos(movementDir);

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

					const Cell& neighbour = getCell(neighbourCol,neighbourRow);

					const bool wallCanContribute 
						= (row == neighbourRow && (getType(row, col - 1) == eFloor || getType(row, col + 1) == eFloor))
						|| (col == neighbourCol && (getType(row - 1, col) == eFloor || getType(row + 1, col) == eFloor));

					const float contribution 
						= neighbour.type == eWall && cell.type == eWall && wallCanContribute == false ? 0.0f
						: (1.0f/8.0f);
					
					heat += getCell(neighbourCol,neighbourRow).fire * contribution;
				}
			}

			const float maxHeat = 1.0f;

			const float heatBuildRate = cell.type == eWall ? 0.25f : 0.05f;

			cell.heat = utils::min( cell.heat + heat * heatBuildRate, maxHeat);

			const float fireThreshold 
				= cell.type == eWall ? 0.3f
				: 0.6f;

			if (cell.heat > fireThreshold)
			{
				cell.fire = utils::mapValue(cell.heat, fireThreshold, maxHeat, 0.0f, 1.0f);
			}
		}
	}
}
