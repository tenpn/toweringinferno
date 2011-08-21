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
				= cell.type == eWall 
					? TCODColor::lerp(TCODColor::lightGrey, TCODColor::darkGrey, 
						utils::mapValue(cell.hp, 0.0f, 1.0f, 0.5f, 1.0f))
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
				: cell.type == eCivilian ? 'd'
				: ' ';

			const float playerHealth = world.getPlayer().getHealth();
			const TCODColor fgColor 
				= isPlayer ? (playerHealth <= 0.2f 
					? TCODColor::lerp(TCODColor::pink, TCODColor::desaturatedOrange, utils::mapValue(playerHealth, 0.0f, 0.2f, 0.0f, 1.0f))
					: TCODColor::lerp(TCODColor::desaturatedOrange, TCODColor::orange, utils::mapValue(playerHealth, 0.2f, 1.0f, 0.0f, 1.0f))
					)
				: cell.type == eCivilian ? TCODColor::violet 
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
	const int highScore,
	const DebugRenderMode renderMode
	)
{
	// hud

	std::stringstream hud;
	if (world.getPlayer().getHealth() == 0.0f)
	{
		hud << "You are dead. Press 'n' or space to restart.";
	}
	else
	{
		hud << "Health:" << static_cast<int>(world.getPlayer().getHealth()*100) << " (B)ombs remaining:" 
			<< world.getPlayer().getBombsRemaining() << " (A)xe strength:" << world.getPlayer().getAxesRemaining() 
			<< " Civilians rescued:" << world.getPlayer().getCiviliansRescued();
	}
	TCODConsole::root->printCenter(world.getWidth()/2, world.getHeight() - 2, TCOD_BKGND_NONE, hud.str().c_str());

	std::stringstream score;
	score << "Floors escaped:" << world.getFloorsEscaped() << " Score:" << world.getPlayer().getScore() 
		<< " High score:" << highScore;
	TCODConsole::root->printCenter(world.getWidth()/2, world.getHeight() - 1, TCOD_BKGND_NONE, score.str().c_str());


	const int mouseX = TCODMouse::getStatus().cx;
	const int mouseY = TCODMouse::getStatus().cy;

	if (world.isValidCoords(mouseX, mouseY))
	{
		const Cell& currentMouseCell = world.getCell(mouseX, mouseY);

		if (renderMode == eDebugRender_Cell)
		{
			std::stringstream waterText;
			waterText << "(" << mouseX << "," << mouseY << ") w:" << currentMouseCell.water << " h:" 
				<< currentMouseCell.heat << " f:" << currentMouseCell.fire << " hp: " << currentMouseCell.hp << " ";
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
			: currentMouseCell.type == eCivilian ? "Walk over civilians to rescue them before they burn or drown."
			: currentMouseCell.water > 0.4f ? "Water quenches fire"
			: currentMouseCell.fire > 0.0f ? "Fire will hurt you a lot"
			: currentMouseCell.heat > 0.25f ? "Hot areas around fire will hurt you a litle"
			: "";

		TCODConsole::root->printCenter(world.getWidth()/2, world.getHeight() - 3, TCOD_BKGND_NONE, tooltip);
	}

	
	// titles

	TCODConsole::root->printCenter(world.getWidth()/2,0,TCOD_BKGND_NONE,"THE TOWERING INFERNO");
	TCODConsole::root->printCenter(world.getWidth()/2,1,TCOD_BKGND_NONE,"Get to the stairs down '>' to escape the floor.");

	static const char* const motd[] = {
		"You are '@'. Move with cursor keys, action with 'a' or enter, space to wait.",
		"'b' drops water bombs.",
		"hover mouse over items for tooltips",
		"Touch civilians 'd' to rescue them before they burn or drown.",
		"The sprinkler control panel 'S' will help control the fire.",
		"Fire hoses 'H' can be turned on to flood small areas.",
		"Closed doors will slow down the fire but also block water",
		"If you're trapped, 'x' will use your axe on the nearest wall",
		"Water bombs work well through a constriction or a doorway",
		"Saving civilians gets you health and bomb bonuses on completing a floor",
		"Water bombs are most effective a few squares away from fire.",
		"If you put water bombs right next to fire, they evaporate too quickly",
	};
	static const int motdCount = sizeof(motd)/sizeof(char*);

	const int helpMessageIndex = (world.getTurnCount() / 3) % motdCount;
	const char* const helpMessage = motd[helpMessageIndex];
	
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

	bool newGamePlease = true;
	bool newFloorPlease = false;
	RenderMode renderMode = eRender_Normal;
	DebugRenderMode debugRenderMode = eDebugRender_None;

	int highestScore = 0;
	World world(width, height);
	
	while ( TCODConsole::isWindowClosed() == false ) 
	{
		if (newFloorPlease || newGamePlease)
		{
			if (newGamePlease)
			{
				world = World(width,height);
			}
			else
			{
				world.resetForNewFloor();
			}

			const int hbuffer = 2;
			const int vbuffer = 3;
			const proceduralgeneration::FloorGenerator floor(
				hbuffer, vbuffer,
				width - hbuffer*2, height - vbuffer*2);

			pushFloorToMap(floor, world);
			newFloorPlease = false;
			newGamePlease = false;

			for(int updateSetup = 0; updateSetup < 3; ++updateSetup)
			{
				TCOD_key_t space = { TCODK_SPACE };
				world.update(space);
			}
			world.resetTurnCount();
		}

		TCODConsole::root->clear();
		renderWorld(world, renderMode);
		debugRender(world, highestScore, debugRenderMode);
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
		
		newFloorPlease = newFloorPlease || (debugRenderMode != eDebugRender_None && key.c == 'w');

		highestScore = utils::max(highestScore, world.getPlayer().getScore());
		
	}
}
