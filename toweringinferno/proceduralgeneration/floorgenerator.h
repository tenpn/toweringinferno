#ifndef __TI_FLOORGENERATOR_H_
#define __TI_FLOORGENERATOR_H_

#include <vector>
#include <assert.h>

namespace toweringinferno
{
	namespace proceduralgeneration
	{

class FloorGenerator
{
public:
	FloorGenerator(int w, int h);
	
	bool isWall(int x, int y) const;
	void setWall(int x, int y);

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

private:

	int coordsToIndex(int x, int y) const;

	std::vector<bool> m_cells;
	int m_width;
	int m_height;
}; 

	} // namespace proceduralgeneration

} // namespace toweringinferno

inline
int toweringinferno::proceduralgeneration::FloorGenerator::coordsToIndex(
	int x, 
	int y
	) const
{
	assert(y >= 0 && y < m_height);
	assert(x >= 0 && x < m_width);
	assert((y*m_width + x) < static_cast<int>(m_cells.size()));
	return y*m_width + x;
}

inline 
void toweringinferno::proceduralgeneration::FloorGenerator::setWall(
	int x, 
	int y
	)
{
	m_cells[coordsToIndex(x,y)] = true;
}

inline
bool toweringinferno::proceduralgeneration::FloorGenerator::isWall(
	int x, 
	int y
	) const
{
	return m_cells[coordsToIndex(x,y)];
}

#endif // __TI_FLOORGENERATOR_H_
