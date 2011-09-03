#ifndef __TI_POSITION_H_
#define __TI_POSITION_H_

#include <utility>

namespace toweringinferno
{
	struct Position
	{
		Position() : col(0), row(0) {}
		Position(const int colIn, const int rowIn) : col(colIn), row(rowIn) {}

		int col;
		int row;
	};

	inline
	bool operator==(const Position& lhs, const Position& rhs)
	{
		return lhs.col == rhs.col && lhs.row == rhs.row;
	}

} // namespace toweringinferno

#endif // __TI_POSITION_H_