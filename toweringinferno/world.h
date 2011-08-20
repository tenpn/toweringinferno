#ifndef __TI_WORLD_H_
#define __TI_WORLD_H_

#include <assert.h>
#include <vector>

namespace toweringinferno
{

enum CellType
{
	eWall,
	eFloor,
	eSky,
};

class World
{
public:
	World(int w, int h);

	CellType get(int x, int y)const;
	void set(int x, int y, CellType newType);

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

private:

	int coordsToIndex(int x, int y) const;

	std::vector<CellType> m_map;
	int m_width;
	int m_height;
}; 

} // namespace toweringinferno

inline
int toweringinferno::World::coordsToIndex(
	const int x, 
	const int y
	) const
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);
	const int index = y * m_width + x;
	assert(index < static_cast<int>(m_map.size()));
	return index;
}

inline
toweringinferno::CellType toweringinferno::World::get(
	const int x,
	const int y
	) const
{
	return m_map[coordsToIndex(x,y)];
}

inline
void toweringinferno::World::set(
	const int x, 
	const int y,
	const CellType newType
	)
{
	m_map[coordsToIndex(x,y)] = newType;
}

#endif // __TI_WORLD_H_