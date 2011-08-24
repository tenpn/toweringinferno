#ifndef __TI_HEATVISION_H_
#define __TI_HEATVISION_H_

#include <vector>
#include "../position.h"

namespace toweringinferno
{

class World;

namespace heatvision
{

struct Civilian
{
	Civilian(const Position& pos) : pos(pos), hp(1.0f) {}
	Civilian() : pos(0,0), hp(1.0f) {}

	Position pos;
	float hp;
};

class HeatvisionSystem
{
public:
	HeatvisionSystem();

	void addCivilian(const Position& pos) { m_civilians.push_back(Civilian(pos)); }

	void update(const World& world);

	typedef std::vector<Civilian> CivilianList;

	const CivilianList& getCivilians() const { return m_civilians; }

private:
	CivilianList m_civilians;
};

} // namespace heatvision
} // namespace toweringinferno

#endif // __TI_HEATVISION_H_