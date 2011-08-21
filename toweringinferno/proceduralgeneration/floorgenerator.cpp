#include <assert.h>
#include "floorgenerator.h"
#include "libtcod.hpp"
#include "../utils.h"

namespace toweringinferno
{
	namespace proceduralgeneration
	{

// -1 if no door
int calculateDoorFrameIndex()
{
	return TCODRandom::getInstance()->getInt(0,4) == 0
		? -1
		: TCODRandom::getInstance()->getInt(1,2);
}

CellType calculateDoorCell(
	const int doorFrameIndex
	)
{
	return doorFrameIndex == 0 
		? (TCODRandom::getInstance()->getInt(0,5) == 0 ? eClosedDoor : eOpenDoor)
		: eFloor;
}

class BSPWallWriter : public ITCODBspCallback 
{
public:
	virtual bool visitNode(TCODBsp * node, void * userData)
	{
		assert(userData != NULL);
		assert(node != NULL);

		FloorGenerator * const floor = static_cast<FloorGenerator*>(userData);

		if (node->isLeaf())
		{
			// make room walls
			assert(node->w > 0);
			assert(node->h > 0);

			const int maxCol = node->x + node->w;
			const int maxRow = node->y + node->h;

			for(int wallCol = node->x; wallCol < maxCol; ++wallCol)
			{
				floor->setType(wallCol, node->y, eWall);
				floor->setType(wallCol, maxRow - 1, eWall);
			}

			for(int wallRow = node->y; wallRow < maxRow; ++wallRow)
			{
				floor->setType(node->x, wallRow, eWall);
				floor->setType(maxCol - 1, wallRow, eWall);
			}
		} 
		else if (node->horizontal) 
		{
			int doorRow = node->position-2;
			const int doorCol = node->x + (node->w / 2);

			int doorFrameIndex = calculateDoorFrameIndex();
			while(doorRow < node->position + 2)
			{
				const CellType doorwayType = calculateDoorCell(doorFrameIndex--);
				floor->setType(doorCol, doorRow++, doorwayType);
			}
		}
		else // vertical split
		{
			int doorCol = node->position-2;
			const int doorRow = node->y + (node->h / 2);

			int doorFrameIndex = calculateDoorFrameIndex();
			while(doorCol < node->position + 2)
			{
				const CellType doorwayType = calculateDoorCell(doorFrameIndex--);
				floor->setType(doorCol++, doorRow, doorwayType);
			}
		}

		return true;
	}
}; 

const TCODBsp& findRandomLeaf(const TCODBsp& node)
{
	return node.isLeaf() ? node
		: TCODRandom::getInstance()->getInt(0,1) == 0 ? findRandomLeaf(*node.getLeft())
		: findRandomLeaf(*node.getRight());
}

Position calculateRandomPosition(const TCODBsp& node)
{
	return Position(
		TCODRandom::getInstance()->getInt(node.x + 1,node.x + node.w - 2),
		TCODRandom::getInstance()->getInt(node.y + 1,node.y + node.h - 2)
		);
}

Position calculateRandomWallPosition(const TCODBsp& node)
{
	const int wallChoice =  TCODRandom::getInstance()->getInt(0,3);

	return wallChoice == 0 ? Position(TCODRandom::getInstance()->getInt(node.x+1,node.x + node.w - 2), node.y)
		: wallChoice == 1 ? Position(TCODRandom::getInstance()->getInt(node.x+1,node.x + node.w - 2), node.y + node.h - 1)
		: wallChoice == 2 ? Position(node.x, TCODRandom::getInstance()->getInt(node.y+1, node.y + node.h - 2))
		: Position(node.x + node.w - 1, TCODRandom::getInstance()->getInt(node.y+1, node.y + node.h - 2));
}

	} // namespace proceduralgeneration
} // namespace toweringinferno

toweringinferno::proceduralgeneration::FloorGenerator::FloorGenerator(
	const int left,
	const int top,
	const int w, 
	const int h, 
	const int floorsCleared
	)
	: m_cells(w*h,eFloor)
	, m_width(w)
	, m_height(h)
	, m_left(left)
	, m_top(top)
{
	TCODBsp officeBsp(left,top,w,h);
	officeBsp.splitRecursive(NULL, 5, 8, 8, 0.9f, 0.9f);

	BSPWallWriter wallWriter;
	officeBsp.traversePostOrder(&wallWriter, this);

	// find player start / end pos by iterating down branches
	const bool startOnLeft = TCODRandom::getInstance()->getInt(0,1) == 0;
	const TCODBsp& playerStartNode = findRandomLeaf(startOnLeft ? *officeBsp.getLeft() : *officeBsp.getRight());
	const Position playerStartPos = calculateRandomPosition(playerStartNode);
	setType(playerStartPos.first, playerStartPos.second, eStairsUp);

	const TCODBsp& playerExitNode = findRandomLeaf(startOnLeft ? *officeBsp.getRight() : *officeBsp.getLeft());
	const Position playerExitPos = calculateRandomPosition(playerExitNode);
	setType(playerExitPos.first, playerExitPos.second, eStairsDown);

	int fireCount = utils::max(2, static_cast<int>((floorsCleared+1)/1.3f));
	assert(fireCount > 0);
	m_initialFires.reserve(fireCount);
	while(fireCount>0)
	{
		const TCODBsp& fireRoom = findRandomLeaf(officeBsp);

		const bool isTooCloseToExitRoom = fireRoom.getFather() == playerExitNode.getFather();

		if (isTooCloseToExitRoom == false)
		{
			m_initialFires.push_back(calculateRandomPosition(fireRoom));

			--fireCount;
		}
	}

	// 2/3rds of time have one hose, only rarely have 2
	int hoseCount = TCODRandom::getInstance()->getInt(1,3) <= 2 ? 1 : 2;
	m_hoses.reserve(hoseCount);
	while(hoseCount > 0)
	{
		const TCODBsp& hoseRoom = findRandomLeaf(officeBsp);

		const Position hosePos = calculateRandomWallPosition(hoseRoom);
		const int cellIndex = worldCoordsToIndex(hosePos.first, hosePos.second);
		if (m_cells[cellIndex] == eWall)
		{
			m_cells[cellIndex] = eHose;
			--hoseCount;
		}
	}

	int sprinklers = 1;
	while(sprinklers > 0)
	{
		const Position sprinklerControlPosition = calculateRandomWallPosition(findRandomLeaf(officeBsp));
		const int sprinklerIndex = worldCoordsToIndex(sprinklerControlPosition.first, sprinklerControlPosition.second);
		if (m_cells[sprinklerIndex] == eWall)
		{
			m_cells[sprinklerIndex] = eSprinklerControl;
			--sprinklers;
		}
	}

	int civilianCount = TCODRandom::getInstance()->getInt(6,8);
	while(civilianCount > 0)
	{
		const Position civilianPosition = calculateRandomPosition(findRandomLeaf(officeBsp));

		const int cellIndex = worldCoordsToIndex(civilianPosition.first, civilianPosition.second);
		if (m_cells[cellIndex] == eFloor)
		{
			m_cells[cellIndex] = eCivilian;
			--civilianCount;
		}
	}
}

