#include "world.h"
#include "utils.h"

toweringinferno::World::World(
	const int w, 
	const int h
	)
	: m_map(w*h)
	, m_width(w)
	, m_height(h)
{

}

void toweringinferno::World::update()
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
