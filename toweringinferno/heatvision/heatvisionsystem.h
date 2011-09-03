#ifndef __TI_HEATVISION_H_
#define __TI_HEATVISION_H_

#include <vector>
#include "../position.h"

namespace toweringinferno
{

class World;

namespace heatvision
{

struct TileHeat
{
	TileHeat() : pos(INT_MAX,INT_MAX), danger(0.0f), desire(0.0f) {}
	TileHeat(const Point posIn, const float dangerIn, const float desireIn)
		: pos(posIn), danger(dangerIn), desire(desireIn)
	{}
	Point pos;
	float danger;
	float desire;
};

enum Tile
{
	eTile_TopLeft,
	eTile_Top,
	eTile_TopRight,
	eTile_Left,
	eTile_Origin,
	eTile_Right,
	eTile_BottomLeft,
	eTile_Bottom,
	eTile_BottomRight,
	eTile_Count,
};

Point calculatePosition(const Point& origin, const Tile tile);

struct Civilian
{
	Civilian(const Point& posIn) : pos(posIn), nextPos(posIn), hp(1.0f) {}
	Civilian() : pos(0,0), nextPos(0,0), hp(1.0f) {}

	Point pos;
	Point nextPos;
	float hp;

	TileHeat heatMap[eTile_Count];

	bool isAlive() const { return hp > 0.0f; }
};

class HeatvisionSystem
{
public:
	HeatvisionSystem();

	void addCivilian(const Point& pos) { m_civilians.push_back(Civilian(pos)); }
	bool tryRemoveCivilian(const Point& pos);

	void preUpdate();
	void update(const World& world);

	typedef std::vector<Civilian> CivilianList;

	const CivilianList& getCivilians() const { return m_civilians; }

private:
	CivilianList m_civilians;
};

inline
bool operator==(
	const Civilian& civvie, 
	const Point& pos
	)
{
	return civvie.pos == pos;
}

} // namespace heatvision
} // namespace toweringinferno

#endif // __TI_HEATVISION_H_