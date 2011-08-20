#include "world.h"

toweringinferno::World::World(
	const int w, 
	const int h
	)
	: m_map(w*h, eSky)
	, m_width(w)
	, m_height(h)
{

}