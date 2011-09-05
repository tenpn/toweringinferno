#include <string>
#include <sstream>
#include <climits>
#include <algorithm>
#include "libtcod.hpp"
#include "game.h"
#include "world.h"
#include "proceduralgeneration/floorgenerator.h"
#include "utils/utils.h"
#include "heatvision/heatvisionsystem.h"

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
		world.setFire(firePos->col, firePos->row, 1.0f);
	}

	for(auto hosePos = floor.getHoses().begin(); hosePos != floor.getHoses().end(); ++hosePos)
	{
		world.setHose(hosePos->col, hosePos->row);
	}
}

void pushFloorToHeatvision(
	const proceduralgeneration::FloorGenerator& floor,
	heatvision::HeatvisionSystem& heatvision
	)
{
	heatvision = heatvision::HeatvisionSystem();
	for(auto civilianPosIt = floor.getCivilians().begin()
		; civilianPosIt != floor.getCivilians().end()
		; ++civilianPosIt)
	{
		heatvision.addCivilian(*civilianPosIt);
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
	const heatvision::HeatvisionSystem& heatvision,
	const RenderMode renderMode,
	const int levelSeed
	)
{
	static const TCODColor fire(255,0,0);
	static const TCODColor heat(255,0,255);
	static const TCODColor water(0,0,255);
	static const TCODColor closedDoor = TCODColor::lerp(TCODColor::darkGrey, TCODColor::grey, 0.5f);

	const TCODColor& renderTargetColor = renderMode == eRender_Heat ? heat : fire;

	TCODRandom floorRng(levelSeed);

	for(int x = 0; x < world.getWidth(); ++x)
	{
		for(int y = 0; y < world.getHeight(); ++y)
		{
			const Cell& cell = world.getCell(x,y);

			if (cell.type == eSky)
			{
				continue;
			}

			const TCODColor floorCol = 
				x % 2 == y % 2
					? TCODColor::lerp(TCODColor::lightGrey, TCODColor::darkGrey, 
						floorRng.getGaussianFloat(0.0f, 0.25f))
				: TCODColor::lightGrey;

			const TCODColor baseBgCol 
				= cell.type == eWall 
					? TCODColor::lerp(TCODColor::lightGrey, TCODColor::darkGrey, 
						utils::mapValue(cell.hp, 0.0f, 1.0f, 0.5f, 1.0f))
				: cell.type == eClosedDoor ? closedDoor
				: cell.type == eSprinklerControl ? 
					(world.isSprinklerSystemAvailable() ? TCODColor::yellow : TCODColor::darkerYellow)
				: floorCol;

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
			
			const bool isPlayer = x == world.getPlayer().getPos().col && y == world.getPlayer().getPos().row;

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

	const TCODColor civilianColour = TCODColor::darkViolet;
	for(auto civilian = heatvision.getCivilians().begin(); civilian != heatvision.getCivilians().end(); ++civilian)
	{
		const TCODColor backgroundCol = TCODConsole::root->getBack(civilian->pos.col, civilian->pos.row);
		TCODConsole::root->putCharEx(civilian->pos.col, civilian->pos.row, 
			'd', civilianColour, backgroundCol);
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
	const heatvision::HeatvisionSystem& heatvision,
	const int highScore,
	const int turnCount,
	const DebugRenderMode renderMode,
	const int levelSeed
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
			<< world.getPlayer().getBombsRemaining() << " A(x)e strength:" << world.getPlayer().getAxesRemaining() 
			<< " Civilians rescued:" << world.getPlayer().getCiviliansRescued();
	}
	TCODConsole::root->printCenter(world.getWidth()/2, world.getHeight() - 2, TCOD_BKGND_NONE, hud.str().c_str());

	std::stringstream score;
	score << "Turns:" << turnCount << " Floors escaped:" << world.getFloorsEscaped() << " Score:" << world.getPlayer().getScore() 
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

			const heatvision::HeatvisionSystem::CivilianList& civilians = heatvision.getCivilians();
			const heatvision::HeatvisionSystem::CivilianList::const_iterator civilianAtMouse
				= std::find(civilians.begin(), civilians.end(), Point(mouseX, mouseY));

			if (civilianAtMouse != civilians.end())
			{
				for(int tileIndex = 0; tileIndex < heatvision::eTile_Count; ++tileIndex)
				{
					const heatvision::TileHeat& heatAtTile = civilianAtMouse->heatMap[tileIndex];
					TCODConsole::root->setBack(heatAtTile.pos.col, heatAtTile.pos.row,
						TCODColor::lerp(TCODColor(0,0,0), TCODColor::red, heatAtTile.danger));
				}
			}
		}

		const char* const tooltip 
			= mouseX == world.getPlayer().getPos().col && mouseY == world.getPlayer().getPos().row
				? "The player"
			: currentMouseCell.type == eOpenDoor || currentMouseCell.type == eClosedDoor 
				? "Close doors with action. Closed doors slow fire and block water."
			: currentMouseCell.type == eHose ? "Open hoses with action. Completely floods nearby rooms."
			: currentMouseCell.type == eSprinklerControl ? "Trigger sprinklers with action. Partially floods whole floor."
			: currentMouseCell.type == eStairsDown ? "Step onto stairs down to escape floor"
			//: currentMouseCell.type == eCivilian ? "Walk over civilians to rescue them before they burn or drown."
			: currentMouseCell.water > 0.4f ? "Water quenches fire"
			: currentMouseCell.fire > 0.0f ? "Fire will hurt you a lot"
			: currentMouseCell.heat > 0.25f ? "Hot areas around fire will hurt you a litle"
			: "";

		TCODConsole::root->printCenter(world.getWidth()/2, world.getHeight() - 3, TCOD_BKGND_NONE, tooltip);
	}

	
	// titles

	std::stringstream titles;
	titles << "THE TOWERING INFERNO";

	if (renderMode == eDebugRender_Cell)
	{
		titles << " seed:" << levelSeed;
	}

	TCODConsole::root->printCenter(world.getWidth()/2,0,TCOD_BKGND_NONE,titles.str().c_str());
	TCODConsole::root->printCenter(world.getWidth()/2,1,TCOD_BKGND_NONE,"Get to the stairs down '>' to escape the floor.");

	static const char* const motd[] = {
		"You are '@'. Move with cursor keys, action with 'a' or enter, space to wait.",
		"TIP: 'b' drops water bombs.",
		"TIP: hover mouse over items for tooltips",
		"TIP: Touch civilians 'd' to rescue them before they burn or drown.",
		"TIP: The sprinkler control panel 'S' will help control the fire.",
		"TIP: Fire hoses 'H' can be turned on to flood small areas.",
		"TIP: Closed doors will slow down the fire but also block water",
		"TIP: If you're trapped, 'x' will use your axe on the nearest wall",
		"TIP: Water bombs work well through a constriction or a doorway",
		"TIP: Saving civilians gets you health and bomb bonuses on completing a floor",
		"TIP: Water bombs are most effective a few squares away from fire.",
		"TIP: If you put water bombs right next to fire, they evaporate too quickly",
	};
	static const int motdCount = sizeof(motd)/sizeof(char*);

	const int helpMessageIndex = (turnCount / 3) % motdCount;
	const char* const helpMessage = motd[helpMessageIndex];
	
	TCODConsole::root->printCenter(world.getWidth()/2,2,TCOD_BKGND_NONE,helpMessage);
}

void removeCivilians(
	Player& player,
	heatvision::HeatvisionSystem& heatVision
	)
{
	if (heatVision.tryRemoveCivilian(player.getPos()))
	{
		player.rescueCivilian();
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
	TCODSystem::setFps(25);

	bool newGamePlease = true;
	bool newFloorPlease = false;
	RenderMode renderMode = eRender_Normal;
	DebugRenderMode debugRenderMode = eDebugRender_None;

	int highestScore = 0;
	int turnCount = 0;
	World world(width, height);
	heatvision::HeatvisionSystem heatvision;
	int levelSeed = 0;
	Point lastExitPosition(-1,-1);
	
	while ( TCODConsole::isWindowClosed() == false ) 
	{
		if (newFloorPlease || newGamePlease)
		{
			if (newGamePlease)
			{
				world = World(width,height);
				turnCount = 0;
			}
			else
			{
				world.resetForNewFloor();
			}

			levelSeed = TCODRandom::getInstance()->getInt(0, INT_MAX);
			const int hbuffer = 2;
			const int vbuffer = 3;
			const proceduralgeneration::FloorGenerator floor(
				levelSeed,
				hbuffer, vbuffer,
				width - hbuffer*2, height - vbuffer*2,
				world.getFloorsEscaped(),
				lastExitPosition);
			lastExitPosition = floor.getExitPosition();

			pushFloorToMap(floor, world);
			pushFloorToHeatvision(floor, heatvision);
			newFloorPlease = false;
			newGamePlease = false;

			for(int updateSetup = 0; updateSetup < 3; ++updateSetup)
			{
				TCOD_key_t space = { TCODK_SPACE };
				world.update(space);
				heatvision.update(world);
			}
		}

		TCODConsole::root->clear();
		renderWorld(world, heatvision, renderMode, levelSeed);
		debugRender(world, heatvision, highestScore, turnCount, debugRenderMode, levelSeed);
		TCODConsole::flush();

		const TCOD_key_t key=TCODConsole::checkForKeypress();
		const WorldEvents ev = world.update(key);
		
		turnCount += (ev == eEvent_InvalidInput || ev == eEvent_PlayerDied) ? 0 : 1;

		if (ev != eEvent_InvalidInput)
		{
			heatvision.preUpdate();
			removeCivilians(world.getPlayer(), heatvision);
			heatvision.update(world);

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
