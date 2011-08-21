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
	, m_health(1.0f)
	, m_waterBombs(2)
{
}

void toweringinferno::Player::update(
	const World& world
	)
{
	m_health = utils::max(m_health - calculateDamage(world.getCell(m_pos)), 0.0f);
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
