
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

		if (node->isLeaf() == false)
		{
			return true;
		}

		FloorGenerator * const floor = static_cast<FloorGenerator*>(userData);

		assert(node->w > 0);
		assert(node->h > 0);

		const int maxCol = node->x + node->w;
		const int maxRow = node->y + node->h;

		for(int wallCol = node->x; wallCol < maxCol; ++wallCol)
		{
			floor->setWall(wallCol, node->y);
			floor->setWall(wallCol, maxRow - 1);
		}

		for(int wallRow = node->y; wallRow < maxRow; ++wallRow)
		{
			floor->setWall(node->x, wallRow);
			floor->setWall(maxCol - 1, wallRow);
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
	officeBsp.splitRecursive(NULL, 4, 8, 8, 0.9f, 0.9f);

	BSPWallWriter wallWriter;
	officeBsp.traverseInOrder(&wallWriter, this);
}
