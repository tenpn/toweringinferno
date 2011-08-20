#include "libtcod.hpp"
#include "game.h"
#include "world.h"
#include "proceduralgeneration\floorgenerator.h"
#include "utils.h"

namespace toweringinferno
{

void pushFloorToMap(
	const proceduralgeneration::FloorGenerator& floor,
	World& world
	)
{
	for(int col = floor.getLeft(); col < floor.getRight(); ++col)
	{
		for(int row = floor.getTop(); row < floor.getBottom(); ++row)
		{
			world.set(col, row, 
				floor.isWall(col, row) ? eWall : eFloor
				);

			//if (col % 5 == 0)
			//{
			//	world.setFire(col, row, 
			//		utils::mapValue(static_cast<float>(row), static_cast<float>(floor.getTop()), static_cast<float>(floor.getBottom()),
			//		0.0f, 1.0f));
			//}
		}
	}
}

void renderWorld(
	const World& world
	)
{
	static const TCODColor fire(255,0,0);

	for(int x = 0; x < world.getWidth(); ++x)
	{
		for(int y = 0; y < world.getHeight(); ++y)
		{
			const Cell& cell = world.getCell(x,y);

			if (cell.type == eSky)
			{
				continue;
			}

			const TCODColor baseBgCol 
				= cell.type == eFloor ? TCODColor::lightGrey
				: cell.type == eWall ? TCODColor::darkGrey
				: TCODColor::lightSky;
			const TCODColor bgCol = TCODColor::lerp(baseBgCol, fire, cell.fire);
			
			const int c
				= cell.type == eWall ? '#'
				: ' ';

			TCODConsole::root->putCharEx(x, y, c, TCODColor::black, bgCol);
		}
	}
}

} // namespace toweringinferno

void toweringinferno::executeGameLoop()
{
	const int width = 80;
	const int height = 50;

	TCODConsole::initRoot(width,height,"inferno",false);
	TCODConsole::root->setBackgroundColor(TCODColor::lightSky);
	TCODConsole::root->setForegroundColor(TCODColor::darkerGrey);

	const int buffer = 2;
	const proceduralgeneration::FloorGenerator floor(
		buffer, buffer,
		width - buffer*2, height - buffer*2);

	World world(width, height);

	pushFloorToMap(floor, world);

	while ( TCODConsole::isWindowClosed() == false ) 
	{
		TCODConsole::root->clear();
		TCODConsole::root->putChar(5,5,'@');
		renderWorld(world);
		TCODConsole::flush();
		const TCOD_key_t key=TCODConsole::waitForKeypress(true);
		
	}
}
