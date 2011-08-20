#include <assert.h>
#include "floorgenerator.h"
#include "libtcod.hpp"


namespace toweringinferno
{
	namespace proceduralgeneration
	{

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

			floor->setType(doorCol, doorRow++, eFloor);
			floor->setType(doorCol, doorRow++, eFloor);
			floor->setType(doorCol, doorRow++, eFloor);
			floor->setType(doorCol, doorRow++, eFloor);
		}
		else // vertical split
		{
			int doorCol = node->position-2;
			const int doorRow = node->y + (node->h / 2);

			floor->setType(doorCol++, doorRow, eFloor);
			floor->setType(doorCol++, doorRow, eFloor);
			floor->setType(doorCol++, doorRow, eFloor);
			floor->setType(doorCol++, doorRow, eFloor);
		}

		return true;
	}
}; 

const TCODBsp& findRandomLeaf(const TCODBsp& node)
{
	return node.isLeaf() ? node
		: TCODRandom::getInstance()->getInt(0,1) == 0 ? *node.getLeft() : *node.getRight();
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

	return wallChoice == 0 ? Position(TCODRandom::getInstance()->getInt(node.x,node.x + node.w - 1), node.y)
		: wallChoice == 1 ? Position(TCODRandom::getInstance()->getInt(node.x,node.x + node.w - 1), node.y + node.h - 1)
		: wallChoice == 2 ? Position(node.x, TCODRandom::getInstance()->getInt(node.y, node.y + node.h - 1))
		: Position(node.x + node.w - 1, TCODRandom::getInstance()->getInt(node.y, node.y + node.h - 1));
}

	} // namespace proceduralgeneration
} // namespace toweringinferno

toweringinferno::proceduralgeneration::FloorGenerator::FloorGenerator(
	const int left,
	const int top,
	const int w, 
	const int h
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

	int fireCount = TCODRandom::getInstance()->getInt(1,3);
	m_initialFires.reserve(fireCount);
	while(fireCount>0)
	{
		const TCODBsp& fireRoom = findRandomLeaf(officeBsp);

		m_initialFires.push_back(calculateRandomWallPosition(fireRoom));

		--fireCount;
	}
}
