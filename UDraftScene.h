#pragma once
#include "UScene.h"

class UDraftScene : public UScene
{
public:
	void Initialize() override;
	void Update(float deltaTime) override;
	void Render(URenderer& renderer) override;
	void Enter() override;

private:
	std::vector<ESkillType> DraftSkills;
};