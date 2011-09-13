
#include "roomgenerator.h"
#include "floorgenerator.h"
#include "libtcod.hpp"

namespace toweringinferno
{
	namespace proceduralgeneration
	{

void createDesk(
	const int x,
	const int y,
	const int w,
	const int h,
	FloorGenerator& floorOut
	)
{
	if (floorOut.getRNG().getInt(0,3) > 0)
	{
		return;
	}

	if (floorOut.getRNG().getInt(0,2) > 0)
	{
		const int row = y + (h/2);
		floorOut.addFurnature(x,row,'h');
		floorOut.addFurnature(x+1,row-1,TCOD_CHAR_NW);
		floorOut.addFurnature(x+1,row,TCOD_CHAR_VLINE);
		floorOut.addFurnature(x+1,row+1,TCOD_CHAR_SW);
		floorOut.addFurnature(x+2,row-1,TCOD_CHAR_NE);
		floorOut.addFurnature(x+2,row,TCOD_CHAR_VLINE);
		floorOut.addFurnature(x+2,row+1,TCOD_CHAR_SE);
	}
	else
	{
		const int col = x + (w/2);
		floorOut.addFurnature(col, y, 'h');
		floorOut.addFurnature(col-1,y+1,TCOD_CHAR_NW);
		floorOut.addFurnature(col,y+1,TCOD_CHAR_HLINE);
		floorOut.addFurnature(col+1,y+1,TCOD_CHAR_NE);
		floorOut.addFurnature(col-1,y+2,TCOD_CHAR_SW);
		floorOut.addFurnature(col,y+2,TCOD_CHAR_HLINE);
		floorOut.addFurnature(col+1,y+2,TCOD_CHAR_SE);
	}
}

void ringBox(
	const int x,
	const int y,
	const int w,
	const int h,
	FloorGenerator& floorOut
	)
{
	const int maxX = x + w;
	const int maxY = y + h;

	for(int col = x; col < maxX; ++col)
	{
		floorOut.addFurnature(col, y, TCOD_CHAR_HLINE);
		floorOut.addFurnature(col, maxY - 1, TCOD_CHAR_HLINE);
	}

	for(int row = y; row < maxY; ++row)
	{
		floorOut.addFurnature(x, row, TCOD_CHAR_VLINE);
		floorOut.addFurnature(maxX - 1, row, TCOD_CHAR_VLINE);
	}

	floorOut.addFurnature(x,y, TCOD_CHAR_NW);
	floorOut.addFurnature(x,maxY - 1, TCOD_CHAR_SW);
	floorOut.addFurnature(maxX - 1,y, TCOD_CHAR_NE);
	floorOut.addFurnature(maxX - 1,maxY - 1, TCOD_CHAR_SE);
}

class BSPDeskWriter : public ITCODBspCallback 
{
public:
	virtual bool visitNode(TCODBsp * node, void * userData)
	{
		if(node->isLeaf() == false)
		{
			return true;
		}

		FloorGenerator * const floor = static_cast<FloorGenerator*>(userData);

		createDesk(node->x, node->y, node->w, node->h, *floor);
		//ringBox(node->x, node->y, node->w, node->h, *floor);

		return true;
	}
};

	} // namespace proceduralgeneration
} // namespace toweringinferno

void toweringinferno::proceduralgeneration::generateRoom(
	const int x,
	const int y,
	const int w,
	const int h,
	FloorGenerator& floorOut
	)
{
	TCODBsp deskBSP(x + 1, y + 1, w - 2, h - 2);
	deskBSP.splitRecursive(&floorOut.getRNG(), 999, 5, 4, 0.95f, 0.95f);

	BSPDeskWriter deskWriter;
	deskBSP.traversePostOrder(&deskWriter, &floorOut);
}