#ifndef __TI_FLOORGENERATOR_H_
#define __TI_FLOORGENERATOR_H_

#include <vector>
#include <assert.h>
#include "../celltype.h"
#include "../position.h"
#include "libtcod.hpp"

namespace toweringinferno
{
	namespace proceduralgeneration
	{

class FloorGenerator
{
public:
	FloorGenerator(int seed, int left, int top, int w, int h, int floorsCleared, const Point& entranceSeed);
	
	CellType getType(int x, int y) const;
	void setType(int x, int y, CellType newType);

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	int getLeft() const { return m_left; }
	int getTop() const { return m_top; }
	int getRight() const { return m_left + m_width; }
	int getBottom() const { return m_top + m_height; }

	typedef std::vector<Point> PositionList;
	const PositionList& getInitialFires() const { return m_initialFires; }
	const PositionList& getHoses() const { return m_hoses; }
	const PositionList& getCivilians() const { return m_civilians; }
	const Point& getExitPosition() const { return m_exitPosition; }

	TCODRandom& getRNG() { return m_rng; }

private:
	bool isWorldCoordsInFloor(int x, int y) const;
	int worldCoordsToIndex(int x, int y) const;

	std::vector<CellType> m_cells;
	int m_width;
	int m_height;
	int m_left;
	int m_top;

	std::vector<Point> m_initialFires;
	std::vector<Point> m_hoses;
	std::vector<Point> m_civilians;

	Point m_exitPosition;

	TCODRandom m_rng;

}; 

	} // namespace proceduralgeneration

} // namespace toweringinferno

inline 
bool toweringinferno::proceduralgeneration::FloorGenerator::isWorldCoordsInFloor(
	const int x, 
	const int y
	) const
{
	assert(x >= 0);
	assert(y >= 0);
	return x >= m_left && x < getRight() && y >= m_top && y < getBottom();
}

inline
int toweringinferno::proceduralgeneration::FloorGenerator::worldCoordsToIndex(
	const int x, 
	const int y
	) const
{
	assert(isWorldCoordsInFloor(x,y));
	const int index = (y - m_top)*m_width + (x - m_left);
	assert(index >= 0 && index < static_cast<int>(m_cells.size()));
	return index;
}

inline 
void toweringinferno::proceduralgeneration::FloorGenerator::setType(
	const int x, 
	const int y,
	const CellType newType
	)
{
	assert(isWorldCoordsInFloor(x,y));
	m_cells[worldCoordsToIndex(x,y)] = newType;
}

inline
toweringinferno::CellType toweringinferno::proceduralgeneration::FloorGenerator::getType(
	const int x, 
	const int y
	) const
{
	return isWorldCoordsInFloor(x,y)
		? m_cells[worldCoordsToIndex(x,y)]
		: eSky;
}

#endif // __TI_FLOORGENERATOR_H_
