#ifndef __TI_POSITION_H_
#define __TI_POSITION_H_

#include <cmath>

namespace toweringinferno
{
	// a point in space
	struct Position
	{
		Position() : col(0), row(0) {}
		Position(const int colIn, const int rowIn) : col(colIn), row(rowIn) {}

		int col;
		int row;
	};

	// a direction in space
	struct Vector
	{
		Vector() : x(0), y(0) {}
		Vector(const int xIn, const int yIn) : x(xIn), y(yIn) {}

		int x;
		int y;
	};

	inline
	bool operator==(const Position& lhs, const Position& rhs)
	{
		return lhs.col == rhs.col && lhs.row == rhs.row;
	}

	inline 
	int calculateManhattenDistance(const Position& lhs, const Position&rhs)
	{
		return abs(lhs.row - rhs.row) + abs(lhs.col - rhs.col);
	}

	inline
	Vector operator-(const Position& lhs, const Position& rhs)
	{
		return Vector(lhs.col - rhs.col, lhs.row - rhs.row);
	}

	inline
	Position operator+(const Position& source, const Vector& direction)
	{
		return Position(source.col + direction.x, source.row + direction.y);
	}

} // namespace toweringinferno

#endif // __TI_POSITION_H_