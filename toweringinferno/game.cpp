#include <string>
#include <sstream>
#include "libtcod.hpp"
#include "game.h"
#include "world.h"
#include "proceduralgeneration/floorgenerator.h"
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
				world.getPlayer().setPos(col, row);
			}
		}
	}

	for(auto firePos = floor.getInitialFires().begin(); firePos != floor.getInitialFires().end(); ++firePos)
	{
		world.setFire(firePos->first, firePos->second, 1.0f);
	}

	for(auto hosePos = floor.getHoses().begin(); hosePos != floor.getHoses().end(); ++hosePos)
	{
		world.setHose(hosePos->first, hosePos->second);
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
	static const TCODColor water(0,0,255);
	static const TCODColor closedDoor = TCODColor::lerp(TCODColor::darkGrey, TCODColor::grey, 0.5f);

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
				: cell.type == eClosedDoor ? closedDoor
				: cell.type == eSprinklerControl ? 
					(world.isSprinklerSystemAvailable() ? TCODColor::yellow : TCODColor::darkerYellow)
				: x % 2 == y % 2
					? TCODColor::lerp(TCODColor::lightGrey, TCODColor::darkGrey, 
						floorRng.getGaussianFloat(0.0f, 0.25f))
				: TCODColor::lightGrey;

			const float renderTarget = renderMode == eRender_Normal ? cell.fire : cell.heat;
			const TCODColor heatBgCol = TCODColor::lerp(baseBgCol, renderTargetColor, 
				renderTarget > 0.0f 
					? utils::clamp(TCODRandom::getInstance()->getGaussianFloat(-0.2f, 0.2f) + renderTarget, 0.0f, 1.0f)
					: 0.0f);

			const TCODColor waterBgCol = TCODColor::lerp(baseBgCol, water, utils::min(cell.water, 1.0f));

			const TCODColor bgCol = (cell.water < cell.fire) 
					|| renderMode == eRender_Heat && cell.heat > 0.0f 
				? heatBgCol
				: waterBgCol;
			
			const bool isPlayer = x == world.getPlayer().getPos().first && y == world.getPlayer().getPos().second;

			const int c
				= isPlayer ? '@'
				: cell.type == eWall ? '#'
				: cell.type == eStairsUp ? '<'
				: cell.type == eStairsDown ? '>'
				: cell.type == eHose ? 'H'
				: cell.type == eClosedDoor ? '+'
				: cell.type == eOpenDoor ? '-'
				: cell.type == eSprinklerControl ? 'S'
				: ' ';

			const float playerHealth = world.getPlayer().getHealth();
			const TCODColor fgColor 
				= isPlayer ? (playerHealth <= 0.2f 
					? TCODColor::lerp(TCODColor::pink, TCODColor::desaturatedOrange, utils::mapValue(playerHealth, 0.0f, 0.2f, 0.0f, 1.0f))
					: TCODColor::lerp(TCODColor::desaturatedOrange, TCODColor::orange, utils::mapValue(playerHealth, 0.2f, 1.0f, 0.0f, 1.0f))
					)
				: cell.type == eHose && cell.hp > 0.0f ? TCODColor::blue
				: cell.type == eStairsDown ? TCODColor::darkGreen
				: TCODColor::black;

			TCODConsole::root->putCharEx(x, y, c, fgColor, bgCol);
		}
	}
}

enum DebugRenderMode
{
	eDebugRender_None,
	eDebugRender_Cell,
	eDebugRender_Count,
};

void debugRender(
	const World& world,
	const DebugRenderMode renderMode
	)
{
	const int mouseX = TCODMouse::getStatus().cx;
	const int mouseY = TCODMouse::getStatus().cy;

	if (world.isValidCoords(mouseX, mouseY))
	{
		const Cell& currentMouseCell = world.getCell(mouseX, mouseY);

		if (renderMode == eDebugRender_Cell)
		{
			std::stringstream waterText;
			waterText << "(" << mouseX << "," << mouseY << ") w:" << currentMouseCell.water << " h:" 
				<< currentMouseCell.heat << " f:" << currentMouseCell.fire;
			TCODConsole::root->printLeft(0, world.getHeight() - 1, TCOD_BKGND_NONE, waterText.str().c_str());
		}

		const char* const tooltip 
			= mouseX == world.getPlayer().getPos().first && mouseY == world.getPlayer().getPos().second
				? "The player"
			: currentMouseCell.type == eOpenDoor || currentMouseCell.type == eClosedDoor 
				? "Close doors with action. Closed doors slow fire and block water."
			: currentMouseCell.type == eHose ? "Open hoses with action. Completely floods nearby rooms."
			: currentMouseCell.type == eSprinklerControl ? "Trigger sprinklers with action. Partially floods whole floor."
			: currentMouseCell.type == eStairsDown ? "Step onto stairs down to escape floor"
			: currentMouseCell.water > 0.4f ? "Water quenches fire"
			: currentMouseCell.fire > 0.0f ? "Fire will hurt you a lot"
			: currentMouseCell.heat > 0.25f ? "Hot areas around fire will hurt you a litle"
			: "";

		TCODConsole::root->printCenter(world.getWidth()/2, world.getHeight() - 3, TCOD_BKGND_NONE, tooltip);
	}

	// hud

	std::stringstream hud;
	if (world.getPlayer().getHealth() == 0.0f)
	{
		hud << "You are dead. Press 'n' or space to restart.";
	}
	else
	{
		hud << "HP: " << static_cast<int>(world.getPlayer().getHealth()*100) << " Bombs remaining: " 
			<< world.getPlayer().getBombsRemaining();
	}
	TCODConsole::root->printCenter(world.getWidth()/2, world.getHeight() - 2, TCOD_BKGND_NONE, hud.str().c_str());

	// titles

	TCODConsole::root->printCenter(world.getWidth()/2,0,TCOD_BKGND_NONE,"THE TOWERING INFERNO");
	TCODConsole::root->printCenter(world.getWidth()/2,1,TCOD_BKGND_NONE,"Get to the stairs down '>' to escape the floor.");

	const char* const helpMessage 
		= world.getTurnCount() < 4 ? "You are '@'. Move with cursor keys, action with 'a' or enter, space to wait."
		: world.getTurnCount() < 8 ? "'b' drops water bombs."
		: world.getTurnCount() < 12 ? "hover mouse over items for tooltips"
		: "";	
	
	TCODConsole::root->printCenter(world.getWidth()/2,2,TCOD_BKGND_NONE,helpMessage);
}

} // namespace toweringinferno

void toweringinferno::executeGameLoop()
{
	const int width = 80;
	const int height = 50;

	TCODConsole::initRoot(width,height,"inferno",false);
	TCODConsole::root->setBackgroundColor(TCODColor::lightSky);
	TCODConsole::root->setForegroundColor(TCODColor::darkerGrey);
	TCODSystem::setFps(25);

	bool newGamePlease = false;
	bool newFloorPlease = true;
	RenderMode renderMode = eRender_Normal;
	DebugRenderMode debugRenderMode = eDebugRender_None;

	World world(width, height);
	
	while ( TCODConsole::isWindowClosed() == false ) 
	{
		if (newFloorPlease || newGamePlease)
		{
			world = World(width,height);

			const int hbuffer = 2;
			const int vbuffer = 3;
			const proceduralgeneration::FloorGenerator floor(
				hbuffer, vbuffer,
				width - hbuffer*2, height - vbuffer*2);

			pushFloorToMap(floor, world);
			newFloorPlease = false;
			newGamePlease = false;
		}

		TCODConsole::root->clear();
		renderWorld(world, renderMode);
		debugRender(world, debugRenderMode);
		TCODConsole::flush();

		const TCOD_key_t key=TCODConsole::checkForKeypress();
		const WorldEvents ev = world.update(key);
		if (ev != eEvent_InvalidInput)
		{
			newFloorPlease = ev == eEvent_NextFloorDown;
			newGamePlease = key.vk == TCODK_SPACE && ev == eEvent_PlayerDied;
		}	
		else if (key.c == 'b')
		{
			world.getPlayer().useWaterBomb(world);
		}
		else if (key.c == 'v')
		{
			renderMode = static_cast<RenderMode>((static_cast<int>(renderMode) + 1) % eRender_Count);
		}
		else if (key.c == 't')
		{
			debugRenderMode = static_cast<DebugRenderMode>((static_cast<int>(debugRenderMode) + 1) % eDebugRender_Count);
		}
		else if (key.c == 'n')
		{
			newGamePlease = true;
		}
		
	}
}
