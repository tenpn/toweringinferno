
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
				floor->setWall(wallCol, node->y, true);
				floor->setWall(wallCol, maxRow - 1, true);
			}

			for(int wallRow = node->y; wallRow < maxRow; ++wallRow)
			{
				floor->setWall(node->x, wallRow, true);
				floor->setWall(maxCol - 1, wallRow, true);
			}
		} 
		else if (node->horizontal) 
		{
			int doorRow = node->position-2;
			const int doorCol = node->x + (node->w / 2);

			floor->setWall(doorCol, doorRow++, false);
			floor->setWall(doorCol, doorRow++, false);
			floor->setWall(doorCol, doorRow++, false);
			floor->setWall(doorCol, doorRow++, false);
		}
		else // vertical split
		{
			int doorCol = node->position-2;
			const int doorRow = node->y + (node->h / 2);

			floor->setWall(doorCol++, doorRow, false);
			floor->setWall(doorCol++, doorRow, false);
			floor->setWall(doorCol++, doorRow, false);
			floor->setWall(doorCol++, doorRow, false);
		}

		return true;
	}
}; 

	} // namespace proceduralgeneration
} // namespace toweringinferno

toweringinferno::proceduralgeneration::FloorGenerator::FloorGenerator(
	const int left,
	const int top,
	const int w, 
	const int h
	)
	: m_cells(w*h,false)
	, m_width(w)
	, m_height(h)
	, m_left(left)
	, m_top(top)
{
	TCODBsp officeBsp(left,top,w,h);
	officeBsp.splitRecursive(NULL, 6, 8, 8, 0.9f, 0.9f);

	BSPWallWriter wallWriter;
	officeBsp.traversePostOrder(&wallWriter, this);
}
