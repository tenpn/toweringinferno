
#include "roomgenerator.h"
#include "floorgenerator.h"
#include "libtcod.hpp"

namespace toweringinferno
{
	namespace proceduralgeneration
	{

enum LineStyle
{
	eLineStyle_Single,
	eLineStyle_Double,
	eLineStyle_Count,
};

enum LineType
{
	eLineType_Horizontal,
	eLineType_Vertical,
	eLineType_Corner_NW,
	eLineType_Corner_NE,
	eLineType_Corner_SW,
	eLineType_Corner_SE,
	eLineType_Count,
};

static unsigned char s_lineConstants[eLineStyle_Count][eLineType_Count] =
{
	{
		TCOD_CHAR_HLINE,
		TCOD_CHAR_VLINE,
		TCOD_CHAR_NW,
		TCOD_CHAR_NE,
		TCOD_CHAR_SW,
		TCOD_CHAR_SE,
	},
	{
		TCOD_CHAR_DHLINE,
		TCOD_CHAR_DVLINE,
		TCOD_CHAR_DNW,
		TCOD_CHAR_DNE,
		TCOD_CHAR_DSW,
		TCOD_CHAR_DSE,
	}
};

void ringBox(
	const int x,
	const int y,
	const int w,
	const int h,
	FloorGenerator& floorOut,
	const LineStyle style
	)
{
	const int maxX = x + w;
	const int maxY = y + h;

	for(int col = x; col < maxX; ++col)
	{
		floorOut.addFurnature(col, y, s_lineConstants[style][eLineType_Horizontal]);
		floorOut.addFurnature(col, maxY - 1, s_lineConstants[style][eLineType_Horizontal]);
	}

	for(int row = y; row < maxY; ++row)
	{
		floorOut.addFurnature(x, row, s_lineConstants[style][eLineType_Vertical]);
		floorOut.addFurnature(maxX - 1, row, s_lineConstants[style][eLineType_Vertical]);
	}

	floorOut.addFurnature(x,y, s_lineConstants[style][eLineType_Corner_NW]);
	floorOut.addFurnature(x,maxY - 1, s_lineConstants[style][eLineType_Corner_SW]);
	floorOut.addFurnature(maxX - 1,y, s_lineConstants[style][eLineType_Corner_NE]);
	floorOut.addFurnature(maxX - 1,maxY - 1, s_lineConstants[style][eLineType_Corner_SE]);
}

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
		ringBox(x+1,row-1,2,3,floorOut, eLineStyle_Single);
	}
	else
	{
		const int col = x + (w/2);
		floorOut.addFurnature(col, y, 'h');
		ringBox(col-1,y+1,3,2, floorOut, eLineStyle_Single);
	}
}

void createBoardroom(
	const int x,
	const int y,
	const int w,
	const int h,
	FloorGenerator& floorOut
	)
{
	// draw table
	ringBox(x+2, y+2, w-4, h-4, floorOut, eLineStyle_Double);

	// surround with chairs
}

class SparseDesksWriter : public ITCODBspCallback 
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
	// todo: cube farm
	// todo: meeting room
	// todo: store room
	// todo: executive office

	if (w > 8 && h > 8 && w < 13 && h < 13 && floorOut.getRNG().getInt(0,1) == 0)
	{
		createBoardroom(x, y, w, h, floorOut);
	}
	else
	{
		TCODBsp deskBSP(x + 1, y + 1, w - 2, h - 2);
		deskBSP.splitRecursive(&floorOut.getRNG(), 999, 5, 4, 0.95f, 0.95f);

		SparseDesksWriter deskWriter;
		deskBSP.traversePostOrder(&deskWriter, &floorOut);
	}
}