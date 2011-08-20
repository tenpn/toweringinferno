#ifndef __TI_UTILS_H_
#define __TI_UTILS_H_

#include <assert.h>

namespace toweringinferno
{
	namespace utils
	{

template<typename T>
inline
T min(
	const T a,
	const T b
	)
{
	return a < b ? a : b;
}

template<typename T>
inline
T max(
	const T a,
	const T b
	)
{
	return a > b ? a : b;
}

template<typename T>
inline
T clamp(
	const T in,
	const T min,
	const T max
	)
{
	assert(min <= max);
	return in < min ? min 
		: in > max ? max
		: in;
}

template<typename T>
inline
T mapValue(
	const T value,
	const T inMin,
	const T inMax,
	const T outMin,
	const T outMax
	)
{
	const T normalisedValue = inMax != inMin
		? (value - inMin) / (inMax - inMin)
		: 0.0f;
	const T unclampedOutValue = normalisedValue * (outMax - outMin) + outMin;
	const T clampedOutValue = outMin < outMax
		? clamp(unclampedOutValue, outMin, outMax)
		: clamp(unclampedOutValue, outMax, outMin);
	return clampedOutValue;
}

	} // namespace utils
} // namespace toweringinferno

#endif // __TI_UTILS_H_
