#include "libtcod.hpp"
#include "game.h"
#include "proceduralgeneration\floorgenerator.h"

namespace toweringinferno
{

void renderFloor(const proceduralgeneration::FloorGenerator& floor)
{
	for(int col = 0; col < floor.getWidth(); ++col)
	{
		for(int row=0; row < floor.getHeight(); ++row)
		{
			if (floor.isWall(col, row))
			{
				TCODConsole::root->putChar(col, row, '#');
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

	const proceduralgeneration::FloorGenerator floor(width, height);

	while ( TCODConsole::isWindowClosed() == false ) 
	{
		TCODConsole::root->clear();
		TCODConsole::root->putChar(5,5,'@');
		renderFloor(floor);
		TCODConsole::flush();
		const TCOD_key_t key=TCODConsole::waitForKeypress(true);
		
	}
}
