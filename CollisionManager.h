#pragma once

#include "UPrimitive.h"

class CollisionManager
{
public :
	bool DetectCollision(UPrimitive* TargetPrimitive, UPrimitive* OtherPrimitive);
	void ResolveCollision(UPrimitive* TargetPrimitive, UPrimitive* OtherPrimitive);
	 
};