#include "libtcod.hpp"
#include "game.h"

void toweringinferno::executeGameLoop()
{
	TCODConsole::initRoot(80,50,"inferno",false);
	TCODConsole::root->setBackgroundColor(TCODColor::lightSky);
	TCODConsole::root->setForegroundColor(TCODColor::darkerGrey);
	while ( TCODConsole::isWindowClosed() == false ) 
	{
		TCODConsole::root->clear();
		TCODConsole::root->putChar(5,5,'@');
		TCODConsole::flush();
		const TCOD_key_t key=TCODConsole::waitForKeypress(true);
		
	}
}
