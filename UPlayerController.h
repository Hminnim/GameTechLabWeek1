#pragma once
#include <vector>
#include "UBall.h"

class UPlayerController
{
private:
	UBall* SelectedBall = nullptr;
	bool bIsDragging = false;

public:
	void Update(std::vector<UPrimitive*>& primitives);
	void UseSkill(ESkillType skillType);
};