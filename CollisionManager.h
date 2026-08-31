#pragma once

#include "UPrimitive.h"

class CollisionManager
{
public :
	void ResolveCollision(UPrimitive* TargetPrimitive, UPrimitive* OtherPrimitive);
};