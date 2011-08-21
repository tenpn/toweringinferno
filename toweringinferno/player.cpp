#include "player.h"
#include "world.h"
#include "utils.h"

namespace toweringinferno
{

float calculateDamage(
	const Cell& cell
	)
{
	return cell.fire > 0.2f 
		? 0.25f
		: utils::mapValue(cell.heat, 0.0f, 1.0f, 0.0f, 0.1f);
}

} // namespace toweringinferno

toweringinferno::Player::Player()
	: m_pos(0,0)
	, m_score(0)
{
}

void toweringinferno::Player::update(
	World& world
	)
{
	m_levelData.health = utils::max(m_levelData.health - calculateDamage(world.getCell(m_pos)), 0.0f);

	const int civiliansRescuedThisTurn = world.rescueCivilian(m_pos) ? 1 : 0;
	m_levelData.civiliansRescued += civiliansRescuedThisTurn;

	if (civiliansRescuedThisTurn > 0)
	{
		m_score += m_levelData.civiliansRescued * m_levelData.civiliansRescued * 100 
			* (1 + world.getFloorsEscaped());
	}
}

void toweringinferno::Player::useWaterBomb(
	World& world
	)
{
	if (m_levelData.waterBombs > 0)
	{
		world.setWaterBomb(getPos());
		--m_levelData.waterBombs;
	}
}

void toweringinferno::Player::resetForNewFloor()
{
	m_levelData = FloorSpecificData();
}

toweringinferno::Player::FloorSpecificData::FloorSpecificData()
	: health(1.0f)
	, waterBombs(2)
	, civiliansRescued(0)
{
}

