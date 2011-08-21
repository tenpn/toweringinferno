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
	, m_waterBombs(8)
	, m_health(1.0f)
{
}

void toweringinferno::Player::update(
	World& world
	)
{
	m_health = utils::max(m_health - calculateDamage(world.getCell(m_pos)), 0.0f);

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
	if (m_waterBombs > 0)
	{
		world.setWaterBomb(getPos());
		--m_waterBombs;
	}
}

void toweringinferno::Player::resetForNewFloor()
{
	typedef std::pair<float,int> HealthAndBombGain;
	static const HealthAndBombGain gains[] = {
		HealthAndBombGain(0.0f, 0),
		HealthAndBombGain(0.2f, 1),
		HealthAndBombGain(0.25f, 2),
		HealthAndBombGain(0.3f, 3),
		HealthAndBombGain(0.1f, 5),
	};
	static const int gainCount = sizeof(gains)/sizeof(HealthAndBombGain);

	const int gainLevel = utils::min(m_levelData.civiliansRescued, gainCount);
	const HealthAndBombGain& gain = gains[gainLevel];

	m_health = utils::min(1.0f, m_health + gain.first);
	m_waterBombs += gain.second;

	m_levelData = FloorSpecificData();
}

toweringinferno::Player::FloorSpecificData::FloorSpecificData()
	: civiliansRescued(0)
{
}

