#include <array>
#include "world.h"
#include "utils/utils.h"


namespace toweringinferno
{

bool isAxeKey(
	const TCOD_key_t& command
	)
{
	return command.vk == TCODK_CHAR && (command.c == 'x' || command.c == 'X');
}

bool isMovementKey(
	const TCOD_keycode_t& key
	)
{
	return key == TCODK_LEFT || key == TCODK_RIGHT || key == TCODK_UP || key == TCODK_DOWN
		|| (key >= TCODK_KP1 && key <= TCODK_KP9);
}

bool isActionKey(
	const TCOD_key_t& command
	)
{
	return (command.vk == TCODK_CHAR && command.c == 'a' || command.c == 'A') 
		|| command.vk == TCODK_ENTER || command.vk == TCODK_KPENTER;
}

bool isDoorToggleKey(
	const TCOD_key_t& command
	)
{
	return isActionKey(command) 
		|| (command.vk == TCODK_CHAR 
			&& (command.c == '=' || command.c == '+' || command.c == 'c' || command.c == 'C'));
}

Point calculateIdealNewPlayerPosition(
	const Point& current,
	const TCOD_keycode_t movementDir
	)
{
	switch(movementDir)
	{
	case TCODK_LEFT:
	case TCODK_KP4:
		return Point(current.col - 1, current.row);

	case TCODK_RIGHT:
	case TCODK_KP6:
		return Point(current.col + 1, current.row);

	case TCODK_UP:
	case TCODK_KP8:
		return Point(current.col, current.row - 1);

	case TCODK_DOWN:
	case TCODK_KP2:
		return Point(current.col, current.row + 1);

	case TCODK_KP7:
		return Point(current.col-1, current.row-1);

	case TCODK_KP9:
		return Point(current.col+1, current.row-1);

	case TCODK_KP1:
		return Point(current.col-1, current.row+1);

	case TCODK_KP3:
		return Point(current.col+1, current.row+1);

	default:
		return current;
	}
}

bool isDeltaWithDirection(
	int deltaX,
	int deltaY,
	const TCOD_keycode_t direction
	)
{
	return deltaX < 0 && deltaY == 0 && (direction == TCODK_LEFT || direction == TCODK_KP4)
		|| deltaX > 0 && deltaY == 0 && (direction == TCODK_RIGHT || direction == TCODK_KP6)
		|| deltaY < 0 && deltaX == 0 && (direction == TCODK_UP || direction == TCODK_KP8)
		|| deltaY > 0 && deltaX == 0 && (direction == TCODK_DOWN || direction == TCODK_KP2)
		|| deltaX < 0 && deltaY < 0 && direction == TCODK_KP7
		|| deltaX > 0 && deltaY < 0 && direction == TCODK_KP9
		|| deltaX < 0 && deltaY > 0 && direction == TCODK_KP1
		|| deltaX > 0 && deltaY > 0 && direction == TCODK_KP3;
}

inline
bool isValidPlayerCell(
	const Cell& cell
	)
{
	return cell.type == eFloor || cell.type == eStairsDown || cell.type == eStairsUp || cell.type == eOpenDoor;
}

inline
bool isHeatProof(
	const CellType cell
	)
{
	return cell == eClosedDoor || cell == eSprinklerControl;
}

inline
bool isWaterBlocker(
	const Cell& cell
	)
{
	return (cell.type == eFloor && cell.hasFurnature() && cell.furnature != 'h') 
		|| cell.type == eSky || cell.type == eWall || cell.type == eClosedDoor || cell.type == eSprinklerControl;
}

} // namespace toweringinferno

toweringinferno::World::World(
	const int w, 
	const int h
	)
	: m_width(w)
	, m_height(h)
	, m_floorData(w, h)
	, m_floorsEscaped(0)
{

}

toweringinferno::World::ActionSuccess toweringinferno::World::calculateNewPlayerPos(
	const TCOD_keycode_t movementDir, 
	const Point& playerPos
	)
{
	if (isMovementKey(movementDir) == false)
	{
		return eAction_InvalidInput;
	}

	m_floorData.lastMovementDir = movementDir;

	const Point idealNewPosition = calculateIdealNewPlayerPosition(playerPos, movementDir);
	const Cell& newPosition = getCell(idealNewPosition);
	if (isValidPlayerCell(newPosition))
	{
		m_player.setPos(idealNewPosition);
		return eAction_Succeeded;
	}
	else
	{
		return eAction_Failed;
	}
}

toweringinferno::WorldEvents toweringinferno::World::update(
	const TCOD_key_t& command
	)
{
	if (isMovementKey(command.vk) == false && isActionKey(command) == false 
		&& isDoorToggleKey(command) == false && isAxeKey(command) == false
		&& command.vk != TCODK_SPACE)
	{
		return eEvent_InvalidInput;
	}

	if (m_player.isDead())
	{
		return eEvent_PlayerDied;
	}

	if ((updateDoors(command) == eAction_Failed 
			&& updateSprinklerControl(command) == eAction_Failed
			&& updateHoseRelease(command) == eAction_Failed)
		|| updateAxe(command) == eAction_Failed
		)
	{
		// the player may have hit it by mistake, ignore it
		return eEvent_InvalidInput;
	}

	if (calculateNewPlayerPos(command.vk, m_player.getPos()) == eAction_Failed)
	{
		return eEvent_InvalidInput;
	}

	m_player.update(*this);
	
	updateDynamics();

	return getType(m_player.getPos()) == eStairsDown ? eEvent_NextFloorDown 
		: eEvent_None;
}

toweringinferno::World::ActionSuccess toweringinferno::World::updateSprinklerControl(
	const TCOD_key_t& command
	)
{
	if (isActionKey(command) == false)
	{
		return eAction_InvalidInput;
	}

	if (m_floorData.isSprinklerAvailable == false)
	{
		return eAction_Failed;
	}

	const Point playerPos = m_player.getPos();
	for(int col = playerPos.col - 1; col < playerPos.col + 2; ++col)
	{
		for(int row = playerPos.row - 1; row < playerPos.row + 2; ++row)
		{
			if (getType(col, row) != eSprinklerControl)
			{
				continue;
			}

			// GO GO SPRINKLERS
			const int sprinklerSpacing = 9;
			for (int sprinklerCol = sprinklerSpacing; sprinklerCol < m_width; sprinklerCol += sprinklerSpacing)
			{
				for(int sprinklerRow = sprinklerSpacing; sprinklerRow < m_height; sprinklerRow += sprinklerSpacing)
				{
					Cell& cell = m_floorData.map[coordsToIndex(sprinklerCol, sprinklerRow)];

					if (cell.type == eFloor)
					{
						cell.water = 1.5f;
						cell.fire = 0.0f;
						cell.heat = 0.0f;
					}
				}
			}

			m_floorData.isSprinklerAvailable = false;
			return eAction_Succeeded;
		}
	}

	return eAction_Failed;
}

toweringinferno::World::ActionSuccess toweringinferno::World::updateHoseRelease(
	const TCOD_key_t& command
	)
{
	if (isActionKey(command) == false)
	{
		return eAction_InvalidInput;
	}

	const Point playerPos = m_player.getPos();
	for(int col = playerPos.col - 1; col < playerPos.col + 2; ++col)
	{
		for(int row = playerPos.row - 1; row < playerPos.row + 2; ++row)
		{
			if (getType(col, row) != eHose)
			{
				continue;
			}

			Cell& hose = m_floorData.map[coordsToIndex(col, row)];
			if (hose.hp == 0.0f)
			{
				continue;
			}

			hose.water = 3.5f;
			hose.hp = 0.0f;
			return eAction_Succeeded;
		}
	}

	return eAction_Failed;
}

toweringinferno::World::ActionSuccess toweringinferno::World::updateAxe(
	const TCOD_key_t& command
	)
{
	if (isAxeKey(command) == false)
	{
		return eAction_InvalidInput;
	}

	if (m_player.getAxesRemaining() == 0)
	{
		return eAction_Failed;
	}

	Cell* wallToAxe = NULL;

	const Point playerPos = m_player.getPos();
	for(int col = playerPos.col - 1; col < playerPos.col + 2; ++col)
	{
		for(int row = playerPos.row - 1; row < playerPos.row + 2; ++row)
		{
			if ((col == playerPos.col && row == playerPos.row) || isValidCoords(col, row) == false)
			{
				continue;
			}

			Cell& axisNeighbourCell = m_floorData.map[coordsToIndex(col, row)];

			if (axisNeighbourCell.type == eWall 
				&& (wallToAxe == NULL 
					|| isDeltaWithDirection(col - playerPos.col, row - playerPos.row, m_floorData.lastMovementDir)))
			{
				wallToAxe = &axisNeighbourCell;
			}
		}
	}

	if (wallToAxe != NULL)
	{
		wallToAxe->hp -= 0.34f;
		if (wallToAxe->hp <= 0.0f)
		{
			wallToAxe->type = eFloor;
			m_player.useAxe();
		}

		return eAction_Succeeded;
	}
	else
	{
		return eAction_Failed;
	}
}

toweringinferno::World::ActionSuccess toweringinferno::World::updateDoors(
	const TCOD_key_t& command
	)
{
	if (isDoorToggleKey(command) == false)
	{
		return eAction_InvalidInput;
	}

	bool didAnyDoorFlip = false;
	const Point playerPos = m_player.getPos();
	for(int col = playerPos.col - 1; col < playerPos.col + 2; ++col)
	{
		for(int row = playerPos.row - 1; row < playerPos.row + 2; ++row)
		{
			if (isValidCoords(col, row) == false)
			{
				continue;
			}

			Cell& axisNeighbourCell = m_floorData.map[coordsToIndex(col, row)];
			if (axisNeighbourCell.type == eOpenDoor)
			{
				axisNeighbourCell.type = eClosedDoor;
				didAnyDoorFlip = true;
			}
			else if (axisNeighbourCell.type == eClosedDoor)
			{
				axisNeighbourCell.type = eOpenDoor;
				didAnyDoorFlip = true;
			}
		}
	}

	return didAnyDoorFlip ? eAction_Succeeded : eAction_Failed;
}

void toweringinferno::World::updateDynamics()
{
	for(int col = 0; col < getWidth(); ++col)
	{
		for(int row = 0; row < getHeight(); ++row)
		{
			Cell& cell = m_floorData.map[coordsToIndex(col, row)];
			cell.typeFlip = cell.type;
		}
	}

	for(int col = 0; col < getWidth(); ++col)
	{
		for(int row = 0; row < getHeight(); ++row)
		{
			Cell& cell = m_floorData.map[coordsToIndex(col, row)];

			if (cell.type == eSky)
			{
				continue;
			}

			// calculate heat
			float heat = 0.0f;
				
			const float minEvaporation = utils::mapValue(cell.water, 0.1f, 0.15f, 0.0f, 0.1f);
			const float evaporation = utils::mapValue(cell.heat, 0.0f, 1.0f, minEvaporation, 0.6f);
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

					Cell& neighbour = m_floorData.map[coordsToIndex(neighbourCol,neighbourRow)];

					const bool isDiagonalNeighbour = row != neighbourRow && col != neighbourCol;

					const bool wallCanContributeHeat 
						= (row == neighbourRow 
							&& getType(col + 1, row) != eClosedDoor && getType(col - 1, row) != eClosedDoor 
							&& ((getType(col, row - 1) == eFloor && getType(col + (neighbourCol < col ? -1 : 1), row - 1) != eClosedDoor) 
								|| (getType(col, row + 1) == eFloor && getType(col + (neighbourCol < col ? -1 : 1), row + 1) != eClosedDoor)))
						|| (col == neighbourCol 
							&& getType(col, row - 1) != eClosedDoor && getType(col, row + 1) != eClosedDoor
							&& ((getType(col - 1, row) == eFloor && getType(col - 1, row + (neighbourRow < row ? -1 : 1)) != eClosedDoor)
								|| (getType(col + 1, row) == eFloor && getType(col + 1, row + (neighbourRow < row ? -1 : 1)) != eClosedDoor)));

					const float heatContribution 
						= isDiagonalNeighbour ? 0.0f
						: neighbour.type == eWall && cell.type == eWall && wallCanContributeHeat == false ? 0.0f
						: neighbour.type == eWall ? (3.5f/6.0f)
						: (1.5f/6.0f);
					
					heat += neighbour.heat * heatContribution;

					if (isDiagonalNeighbour == false)
					{
						condensationScore += neighbour.water;
						condensationContributors += neighbour.water > 0.0f ? 1 : 0;
					}

					const bool isNeighbourContributingWater = isWaterBlocker(neighbour) == false
						&& cell.water < neighbour.water;
					waterTotal += (isNeighbourContributingWater ? neighbour.water : 0.0f);
					waterContributors += isNeighbourContributingWater ? 1 : 0;
				}
			}

			cell.waterFlip = isWaterBlocker(cell) ? 0.0f
				: utils::max(waterTotal / static_cast<float>(waterContributors), 0.0f);
			
			const float heatBuildRate 
				= cell.type == eWall ? 0.65f 
				: isHeatProof(cell.type) ? 0.0f
				: 0.25f;

			const float condensation = utils::mapValue(condensationScore / static_cast<float>(condensationContributors), 
				0.0f, 1.0f, 0.01f, 0.6f);
			cell.heatFlip = utils::clamp( cell.heat + heat * heatBuildRate - condensation, 0.0f, 1.0f);

			if (cell.type == eWall)
			{
				cell.hp = utils::max(0.0f, cell.hp - utils::mapValue(cell.fire, 0.9f, 1.0f, 0.0f, 0.02f));

				if (cell.hp == 0.0f)
				{
					cell.typeFlip = eFloor;
					cell.fire = 0.75f;
				}
			}
		}
	}

	for(int col = 0; col < getWidth(); ++col)
	{
		for(int row = 0; row < getHeight(); ++row)
		{
			Cell& cell = m_floorData.map[coordsToIndex(col, row)];
			cell.type = cell.typeFlip;

			cell.water = cell.waterFlip;

			cell.heat = cell.heatFlip;

			const float fireThreshold = 0.5f;

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

void toweringinferno::World::resetForNewFloor()
{
	m_floorData = FloorSpecificData(m_width, m_height);
	++m_floorsEscaped;
	m_player.resetForNewFloor(m_floorsEscaped);
}

toweringinferno::World::FloorSpecificData::FloorSpecificData(
	const int w, 
	const int h
	)
	: map(w*h)
	, isSprinklerAvailable(true)
	, lastMovementDir(TCODK_NONE)
{
}
