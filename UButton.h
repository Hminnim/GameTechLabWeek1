#pragma once
#include "UUI.h"
#include "UGameManager.h"
#include "USoundManager.h"
#include <functional>

// 클릭 가능한 UI (UUI 상속)
class UButton : public UUI
{
public:
    // 클릭됐을 때 실행할 함수
    void SetOnClick(std::function<void()> callback) { _onClick = callback; }

    // 마우스 좌표 영역 판정
    bool HitTest(float mouseX, float mouseY) const;

    // 클릭 판정 성공
    virtual void OnClick();

    virtual void PlayClickSound() { USoundManager::GetInstance().PlaySound("select_menu"); }

    virtual void Render(URenderer& renderer) override;
        
private:
    std::function<void()> _onClick;
};


enum class ESkillButtonState
{
    Normal,
    Hovered,
    Selected,
    Used
};

class USkillButton : public UButton
{
public:
    void SetUsedTexture(const std::string& textureKey) { _usedTextureKey = textureKey; }
    void SetSkillType(ESkillType skillType) { _skillType = skillType; }
	void SetPosition(float x, float y) { _x = x; _y = y; }

    virtual void OnClick() override;

	virtual void PlayClickSound() override { USoundManager::GetInstance().PlaySound("select_item"); }

    virtual void Update(float deltaTime) override;
    virtual void Render(URenderer& renderer) override;

private:
	ESkillType _skillType = ESkillType::None;
    ESkillButtonState _state = ESkillButtonState::Normal;
    std::string _usedTextureKey;
    char _skillAuraKey = 0; // Skill별 Aura Effect 구분 key
};