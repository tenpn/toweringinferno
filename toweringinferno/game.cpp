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

enum RenderMode
{
	eRender_Normal,
	eRender_Heat,
	eRender_Count,
};

void renderWorld(
	const World& world,
	const RenderMode renderMode
	)
{
	static const TCODColor fire(255,0,0);
	static const TCODColor heat(255,0,255);

	const TCODColor& renderTargetColor = renderMode == eRender_Heat ? heat : fire;

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

			const float renderTarget = renderMode == eRender_Normal ? cell.fire : cell.heat;
			const TCODColor bgCol = TCODColor::lerp(baseBgCol, renderTargetColor, 
				renderTarget > 0.0f 
					? utils::clamp(TCODRandom::getInstance()->getGaussianFloat(-0.2f, 0.2f) + renderTarget, 0.0f, 1.0f)
					: 0.0f);
			
			const bool isPlayer = x == world.getPlayerPos().first && y == world.getPlayerPos().second;

			const int c
				= isPlayer ? '@'
				: cell.type == eWall ? '#'
				: cell.type == eStairsUp ? '<'
				: cell.type == eStairsDown ? '>'
				: ' ';

			const TCODColor fgColor = isPlayer 
				? (world.getPlayerHealth() <= 0.2f 
					? TCODColor::lerp(TCODColor::pink, TCODColor::desaturatedOrange, utils::mapValue(world.getPlayerHealth(), 0.0f, 0.2f, 0.0f, 1.0f))
					: TCODColor::lerp(TCODColor::desaturatedOrange, TCODColor::orange, utils::mapValue(world.getPlayerHealth(), 0.2f, 1.0f, 0.0f, 1.0f))
					)
				: TCODColor::black;

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

	bool newGamePlease = false;
	bool newFloorPlease = true;
	RenderMode renderMode = eRender_Normal;

	World world(width, height);

	while ( TCODConsole::isWindowClosed() == false ) 
	{
		if (newFloorPlease || newGamePlease)
		{
			world = World(width,height);

			const int buffer = 2;
			const proceduralgeneration::FloorGenerator floor(
				buffer, buffer,
				width - buffer*2, height - buffer*2);

			pushFloorToMap(floor, world);
			newFloorPlease = false;
			newGamePlease = false;
		}

		TCODConsole::root->clear();
		renderWorld(world, renderMode);
		TCODConsole::flush();
		const TCOD_key_t key=TCODConsole::waitForKeypress(true);
		if (key.vk == TCODK_LEFT || key.vk == TCODK_RIGHT || key.vk == TCODK_UP || key.vk == TCODK_DOWN 
			|| key.vk == TCODK_SPACE)
		{
			const WorldEvents ev = world.update(key.vk);
			newFloorPlease = ev == eEvent_NextFloorDown;
			newGamePlease = key.vk == TCODK_SPACE && ev == eEvent_PlayerDied;
		}	
		else if (key.c == 'v')
		{
			renderMode = static_cast<RenderMode>((static_cast<int>(renderMode) + 1) % eRender_Count);
		}
		
	}
}
