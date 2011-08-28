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
	TileHeat(const Position posIn, const float dangerIn, const float desireIn)
		: pos(posIn), danger(dangerIn), desire(desireIn)
	{}
	Position pos;
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

Position calculatePosition(const Position& origin, const Tile tile);

struct Civilian
{
	Civilian(const Position& posIn) : pos(posIn), nextPos(posIn), hp(1.0f) {}
	Civilian() : pos(0,0), nextPos(0,0), hp(1.0f) {}

	Position pos;
	Position nextPos;
	float hp;

	TileHeat heatMap[eTile_Count];
};

class HeatvisionSystem
{
public:
	HeatvisionSystem();

	void addCivilian(const Position& pos) { m_civilians.push_back(Civilian(pos)); }
	bool tryRemoveCivilian(const Position& pos);

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
	const Position& pos
	)
{
	return civvie.pos == pos;
}

} // namespace heatvision
} // namespace toweringinferno

#endif // __TI_HEATVISION_H_