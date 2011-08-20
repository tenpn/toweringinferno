
#include "floorgenerator.h"

toweringinferno::proceduralgeneration::FloorGenerator::FloorGenerator(
	const int w, 
	const int h
	)
	: m_cells(w*h,false)
	, m_width(w)
	, m_height(h)
{
	// sample wall
	for (int col = 0; col < w; ++col)
	{
		m_cells[coordsToIndex(col,(h/2))] = true;
	}
}
