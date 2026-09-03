#pragma once
#include "UScene.h"

class UDraftScene : public UScene
{
public:
	void Initialize() override;
	void Update(float deltaTime) override;
	void Render(URenderer& renderer) override;
	void Enter() override;

	void AddDraftedUI(UUI* draftedUI) { m_draftedUIs.push_back(draftedUI); }
	void SetDraftedUI(ESkillType skillType);

private:
	bool m_bisNowEnter = true;
	bool m_bIsFadingOut = false;

	std::vector<ESkillType> DraftSkills;
	std::vector<UUI*> m_draftedUIs;

	UFadeOverlay m_fadeoverlay;
};