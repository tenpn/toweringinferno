#include <algorithm>
#include "heatvisionsystem.h"
#include "../world.h"

namespace toweringinferno
{
namespace heatvision
{

bool operator==(const Civilian& civvie, const Position& pos)
{
	return civvie.pos == pos;
}

} // namespace heatvision
} // namespace toweringinferno

toweringinferno::heatvision::HeatvisionSystem::HeatvisionSystem()
{
}

void toweringinferno::heatvision::HeatvisionSystem::update(
	const toweringinferno::World& /*world*/
	)
{
}

bool toweringinferno::heatvision::HeatvisionSystem::tryRemoveCivilian(
	const Position& pos
	)
{
	const CivilianList::iterator civilian = std::find(m_civilians.begin(), m_civilians.end(), pos);
	if (civilian != m_civilians.end())
	{
		m_civilians.erase(civilian);
		return true;
	}
	return false;
}
