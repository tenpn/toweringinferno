#include "libtcod.hpp"
#include "game.h"
#include "proceduralgeneration\floorgenerator.h"

namespace toweringinferno
{

void renderFloor(
	const proceduralgeneration::FloorGenerator& floor
	)
{
	for(int col = floor.getLeft(); col < floor.getRight(); ++col)
	{
		for(int row = floor.getTop(); row < floor.getBottom(); ++row)
		{
			TCODConsole::root->setBack(col, row, TCODColor::lightGrey);
			if (floor.isWall(col, row))
			{
				TCODConsole::root->putCharEx(col, row, '#', TCODColor::darkerGrey, TCODColor::lightGrey);
			}
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

	while ( TCODConsole::isWindowClosed() == false ) 
	{
		TCODConsole::root->clear();
		TCODConsole::root->putChar(5,5,'@');
		renderFloor(floor);
		TCODConsole::flush();
		const TCOD_key_t key=TCODConsole::waitForKeypress(true);
		
	}
}
