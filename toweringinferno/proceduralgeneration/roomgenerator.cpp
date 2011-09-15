#include <cassert>
#include <cmath>
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

enum FillStyle
{
	eFillStyle_Filled,
	eFillStyle_Empty,
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
	const LineStyle line,
	const FillStyle fill
	)
{
	const int maxX = x + w;
	const int maxY = y + h;

	for(int col = x+1; col < maxX-1; ++col)
	{
		floorOut.addFurnature(col, y, s_lineConstants[line][eLineType_Horizontal]);
		floorOut.addFurnature(col, maxY - 1, s_lineConstants[line][eLineType_Horizontal]);

		if (fill == eFillStyle_Filled)
		{
			for(int row = y+1; row < maxY-1; ++row)
			{
				floorOut.addFurnature(col, row, '.');
			}
		}
	}

	for(int row = y+1; row < maxY-1; ++row)
	{
		floorOut.addFurnature(x, row, s_lineConstants[line][eLineType_Vertical]);
		floorOut.addFurnature(maxX - 1, row, s_lineConstants[line][eLineType_Vertical]);
	}

	floorOut.addFurnature(x,y, s_lineConstants[line][eLineType_Corner_NW]);
	floorOut.addFurnature(x,maxY - 1, s_lineConstants[line][eLineType_Corner_SW]);
	floorOut.addFurnature(maxX - 1,y, s_lineConstants[line][eLineType_Corner_NE]);
	floorOut.addFurnature(maxX - 1,maxY - 1, s_lineConstants[line][eLineType_Corner_SE]);
}

enum ChairOrientation
{
	eChairOrientation_Left,
	eChairOrientation_Top,
};

void createDesk(
	const int x,
	const int y,
	const int w,
	const int h,
	const ChairOrientation chairOrientation,
	FloorGenerator& floorOut
	)
{
	assert (w >= 3);
	assert (h >= 3);

	if (chairOrientation == eChairOrientation_Left)
	{
		const int row = y + (h/2);
		floorOut.addFurnature(x,row,'h');
		ringBox(x+1,row-1,2,3,floorOut, eLineStyle_Single, eFillStyle_Empty);
	}
	else
	{
		const int col = x + (w/2);
		floorOut.addFurnature(col, y, 'h');
		ringBox(col-1,y+1,3,2, floorOut, eLineStyle_Single, eFillStyle_Empty);
	}
}

void createCubeFarm(
	const int x,
	const int y,
	const int w,
	const int h,
	const ChairOrientation chairOrientation,
	FloorGenerator& floorOut
	)
{
	const int cubeFarmDeskWidth = 4;
	assert(w >= cubeFarmDeskWidth);
	assert(h >= cubeFarmDeskWidth);
	const int desksWide = static_cast<int>(floor(w/static_cast<float>(cubeFarmDeskWidth)));
	const int desksTall = static_cast<int>(floor(h/static_cast<float>(cubeFarmDeskWidth)));

	for(int deskX = 0; deskX < desksWide; ++deskX)
	{
		for(int deskY = 0; deskY < desksTall; ++deskY)
		{
			createDesk(x + cubeFarmDeskWidth*deskX, y + cubeFarmDeskWidth*deskY, cubeFarmDeskWidth, cubeFarmDeskWidth,
				chairOrientation, floorOut);
		}
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
	// make sure desk dimensions are always odd
	const int deskColBegin = x+2;
	const int deskWidth = w%2 == 0 ? w-5 : w-4; 
	const int deskRowBegin = y+2;
	const int deskHeight = h%2 == 0 ? h-5 : h-4; 
	ringBox(deskColBegin, deskRowBegin, deskWidth, deskHeight, floorOut, eLineStyle_Double, eFillStyle_Filled);

	// surround with chairs
	const int deskColEnd = deskColBegin + deskWidth;
	const int chairSpacing = 2;
	const int chairSpacingMid = static_cast<int>(chairSpacing/2);
	const int deskRowEnd = deskRowBegin + deskHeight;
	for(int col = deskColBegin; col < deskColEnd; ++col)
	{
		if ((col - deskColBegin) % chairSpacing == chairSpacingMid)
		{
			floorOut.addFurnature(col, deskRowBegin-1, 'h');
			floorOut.addFurnature(col, deskRowEnd, 'h');
		}
	}

	for(int row = y+2; row < deskRowEnd; ++row)
	{
		if ((row - deskRowBegin) % chairSpacing == chairSpacingMid)
		{
			floorOut.addFurnature(deskColBegin-1, row, 'h');
			floorOut.addFurnature(deskColEnd, row, 'h');
		}
	}
}

class DesksWriter : public ITCODBspCallback 
{
public:
	DesksWriter(const float proportionFilledLeaves, const float proportionLeftChairs)
		: m_proportionFilledLeaves(proportionFilledLeaves)
		, m_proportionLeftChairs(proportionLeftChairs)
	{}

	virtual bool visitNode(TCODBsp * node, void * userData)
	{
		if(node->isLeaf() == false)
		{
			return true;
		}

		FloorGenerator * const floor = static_cast<FloorGenerator*>(userData);

		if (floor->getRNG().getFloat(0.0f, 1.0f) <= m_proportionFilledLeaves)
		{
			const ChairOrientation chair = floor->getRNG().getFloat(0.0f, 1.0f) <= m_proportionLeftChairs 
				? eChairOrientation_Left 
				: eChairOrientation_Top;
			createDesk(node->x, node->y, node->w, node->h, chair, *floor);
		}
		return true;
	}

private:
	float m_proportionFilledLeaves;
	float m_proportionLeftChairs;
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
	else if (w > 10 && h > 10 && floorOut.getRNG().getInt(0,2) == 0)
	{
		createCubeFarm(x+1, y+1, w-1, h-1, 
			floorOut.getRNG().getInt(0,1) == 0 ? eChairOrientation_Left : eChairOrientation_Top, 
			floorOut);
	}
	else
	{
		// just a few desks
		TCODBsp deskBSP(x + 1, y + 1, w - 2, h - 2);
		deskBSP.splitRecursive(&floorOut.getRNG(), 999, 5, 4, 0.95f, 0.95f);

		DesksWriter sparseDesksWriter(0.25f, 0.66f);
		deskBSP.traversePostOrder(&sparseDesksWriter, &floorOut);
	}
}