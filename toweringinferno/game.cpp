#include "libtcod.hpp"
#include "game.h"
#include "world.h"
#include "proceduralgeneration\floorgenerator.h"

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
		}
	}
}

void renderWorld(
	const World& world
	)
{
	for(int x = 0; x < world.getWidth(); ++x)
	{
		for(int y = 0; y < world.getHeight(); ++y)
		{
			const CellType type = world.get(x,y);

			if (type == eSky)
			{
				continue;
			}

			const TCODColor bgCol 
				= type == eFloor ? TCODColor::lightGrey
				: type == eWall ? TCODColor::darkGrey
				: TCODColor::lightSky;
			const int c
				= type == eWall ? '#'
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
