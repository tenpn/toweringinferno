#ifndef __TI_ROOMGENERATOR_H_
#define __TI_ROOMGENERATOR_H_

namespace toweringinferno
{
	namespace proceduralgeneration
	{

class FloorGenerator;

void generateRoom(int x, int y, int w, int h, FloorGenerator& floorOut);

	}
}

#endif // __TI_ROOMGENERATOR_H_
