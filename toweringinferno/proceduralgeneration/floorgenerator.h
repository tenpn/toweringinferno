#ifndef __TI_FLOORGENERATOR_H_
#define __TI_FLOORGENERATOR_H_

#include <vector>
#include <assert.h>
#include "..\celltype.h"

namespace toweringinferno
{
	namespace proceduralgeneration
	{

class FloorGenerator
{
public:
	FloorGenerator(int left, int top, int w, int h);
	
	bool isWall(int x, int y) const;
	void setWall(int x, int y, bool wall);

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	int getLeft() const { return m_left; }
	int getTop() const { return m_top; }
	int getRight() const { return m_left + m_width; }
	int getBottom() const { return m_top + m_height; }

private:
	bool isWorldCoordsInFloor(int x, int y) const;
	int worldCoordsToIndex(int x, int y) const;

	std::vector<CellType> m_cells;
	int m_width;
	int m_height;
	int m_left;
	int m_top;
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
void toweringinferno::proceduralgeneration::FloorGenerator::setWall(
	const int x, 
	const int y,
	const bool wall
	)
{
	assert(isWorldCoordsInFloor(x,y));
	m_cells[worldCoordsToIndex(x,y)] = wall ? eWall : eFloor;
}

inline
bool toweringinferno::proceduralgeneration::FloorGenerator::isWall(
	const int x, 
	const int y
	) const
{
	return isWorldCoordsInFloor(x,y)
		? m_cells[worldCoordsToIndex(x,y)] == eWall
		: false;
}

#endif // __TI_FLOORGENERATOR_H_
