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
				floor.getType(col, row)
				);

			if (floor.getType(col, row) == eStairsUp)
			{
				world.setPlayerPos(col, row);
			}
		}
	}

	for(auto firePos = floor.getInitialFires().begin(); firePos != floor.getInitialFires().end(); ++firePos)
	{
		world.setFire(firePos->first, firePos->second, 1.0f);
	}
}

void renderWorld(
	const World& world
	)
{
	static const TCODColor fire(255,0,0);

	TCODRandom floorRng(0);

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
				= cell.type == eWall ? TCODColor::darkGrey
				: x % 2 == y % 2
					? TCODColor::lerp(TCODColor::lightGrey, TCODColor::darkGrey, 
						floorRng.getGaussianFloat(0.0f, 0.25f))
				: TCODColor::lightGrey;
			const TCODColor bgCol = TCODColor::lerp(baseBgCol, fire, 
				cell.fire > 0.0f 
					? utils::clamp(TCODRandom::getInstance()->getGaussianFloat(-0.2f, 0.2f) + cell.fire, 0.0f, 1.0f)
					: 0.0f);
			
			const bool isPlayer = x == world.getPlayerPos().first && y == world.getPlayerPos().second;

			const int c
				= isPlayer ? '@'
				: cell.type == eWall ? '#'
				: cell.type == eStairsUp ? '<'
				: cell.type == eStairsDown ? '>'
				: ' ';

			const TCODColor fgColor = isPlayer ? TCODColor::orange : TCODColor::black;

			TCODConsole::root->putCharEx(x, y, c, fgColor, bgCol);
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

	bool newFloorPlease = true;

	World world(width, height);

	while ( TCODConsole::isWindowClosed() == false ) 
	{
		if (newFloorPlease)
		{
			world = World(width,height);

			const int buffer = 2;
			const proceduralgeneration::FloorGenerator floor(
				buffer, buffer,
				width - buffer*2, height - buffer*2);

			pushFloorToMap(floor, world);
			newFloorPlease = false;
		}

		TCODConsole::root->clear();
		renderWorld(world);
		TCODConsole::flush();
		const TCOD_key_t key=TCODConsole::waitForKeypress(true);
		if (key.vk == TCODK_LEFT || key.vk == TCODK_RIGHT || key.vk == TCODK_UP || key.vk == TCODK_DOWN 
			|| key.vk == TCODK_SPACE)
		{
			newFloorPlease = world.update(key.vk) == eEvent_NextFloorDown;
		}	
		
	}
}
