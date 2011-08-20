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

struct Cell
{
	Cell() : type(eSky), fire(0.0f), heat(0.0f) {}

	CellType type;
	float fire;
	float heat;

	void setFire(const float fireIn) { fire = heat = fireIn; }
};

class World
{
public:
	World(int w, int h);

	void update();

	CellType getType(int x, int y)const;
	const Cell& getCell(int x, int y)const;
	void set(int x, int y, CellType newType);
	void setFire(int x, int y, float newFire);

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

private:

	int coordsToIndex(int x, int y) const;

	std::vector<Cell> m_map;
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
const toweringinferno::Cell& toweringinferno::World::getCell(
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
	m_map[coordsToIndex(x,y)].type = newType;
}

inline
void toweringinferno::World::setFire(
	const int x, 
	const int y, 
	const float newFire
	)
{
	m_map[coordsToIndex(x,y)].fire = newFire;
}

inline
toweringinferno::CellType toweringinferno::World::getType(
	const int x, 
	const int y
	)const
{
	return x < 0 || x >= m_width ? eSky
		: y < 0 || y >= m_height ? eSky
		: m_map[coordsToIndex(x,y)].type;
}

#endif // __TI_WORLD_H_