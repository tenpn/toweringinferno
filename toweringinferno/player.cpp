#include <math.h>
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

struct Rewards
{
	Rewards(float _healthDelta, int _bombDelta, int _axeDelta)
		: healthDelta(_healthDelta), bombDelta(_bombDelta), axeDelta(_axeDelta)
	{}

	float healthDelta;
	int bombDelta;
	int axeDelta;
};

} // namespace toweringinferno

toweringinferno::Player::Player()
	: m_pos(0,0)
	, m_score(0)
	, m_waterBombs(4)
	, m_health(1.0f)
	, m_axeCount(4)
{
}

void toweringinferno::Player::update(
	World& world
	)
{
	m_health = utils::max(m_health - calculateDamage(world.getCell(m_pos)), 0.0f);

	const int civiliansRescuedThisTurn = world.rescueCivilian(m_pos) ? 1 : 0;
	m_levelData.civiliansRescued += civiliansRescuedThisTurn;
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

void toweringinferno::Player::resetForNewFloor(
	const int floorsCleared
	)
{
	static const Rewards gains[] = {
		Rewards(0.0f, 0, 0),
		Rewards(0.2f, 1, 0),
		Rewards(0.25f, 2, 1),
		Rewards(0.3f, 2, 2),
		Rewards(0.3f, 3, 2),
		Rewards(0.4f, 4, 3),
	};
	static const int gainCount = sizeof(gains)/sizeof(Rewards);

	const int gainLevel = utils::min(m_levelData.civiliansRescued, gainCount-1);
	const Rewards& gain = gains[gainLevel];

	m_health = utils::min(1.0f, m_health + gain.healthDelta);
	m_waterBombs = utils::min(m_waterBombs + gain.bombDelta, 6);
	m_axeCount = utils::min(m_axeCount + gain.axeDelta, 5);

	m_score += floorsCleared * 100 * static_cast<int>(pow(2.0f, m_levelData.civiliansRescued));

	m_levelData = FloorSpecificData();
}

toweringinferno::Player::FloorSpecificData::FloorSpecificData()
	: civiliansRescued(0)
{
}

