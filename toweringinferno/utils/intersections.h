#ifndef __TI_INTERSECTIONS_H_
#define __TI_INTERSECTIONS_H_

#include "../position.h"

namespace toweringinferno
{
namespace utils
{

class Circle
{
public:
	Circle(const Point& origin, const int radius) : m_origin(origin), m_radius(radius) {}

	bool contains(const Point& p)const;	

private:
	Point m_origin;
	int m_radius;
};

inline
bool toweringinferno::utils::Circle::contains(const Point& p)const
{
	return calculateManhattenDistance(p, m_origin) <= m_radius;
}

} // namespace utils
} // namespace toweringinferno

#endif // __TI_INTERSECTIONS_H_