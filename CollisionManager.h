#pragma once

class UPrimitive;


class CollisionManager
{
public :
	static CollisionManager& GetInstance() {
		static CollisionManager Instance;
		return Instance;
	}

	CollisionManager();
	~CollisionManager();
	CollisionManager(const CollisionManager&) = delete;
	CollisionManager& operator=(const CollisionManager&) = delete;

	bool DetectCollision(UPrimitive* TargetPrimitive, UPrimitive* OtherPrimitive);
	void ResolveCollision(UPrimitive* TargetPrimitive, UPrimitive* OtherPrimitive);
	 
};