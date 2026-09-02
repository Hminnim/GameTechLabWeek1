#pragma once
#include "pch.h"
#include "UButton.h"
#include "URenderer.h"
#include "UResourceManager.h"
#include "UGameManager.h"
#include "UGameSetting.h"
#include "UEffectManager.h"

bool UButton::HitTest(float mouseX, float mouseY) const
{
	if (!_isActive)
		return false;
    return (mouseX >= _x && mouseX <= _x + _width) &&
        (mouseY >= _y && mouseY <= _y + _height);
}

void UButton::OnClick()
{
    if (_onClick)
    {
        _onClick();
    }
}

void UButton::Render(URenderer& renderer)
{
	UUI::Render(renderer);
}

//////////////////
// USkillButton //
//////////////////
void USkillButton::OnClick()
{
    UButton::OnClick();

	if (_state == ESkillButtonState::Normal)
	{
		_state = ESkillButtonState::Selected;
		UGameManager::GetInstance().m_currentSelectedSkill = _skillType;
	}
}

void USkillButton::Update(float deltaTime)
{
	if (UGameManager::GetInstance().m_usedSkills[UGameManager::GetInstance().CurrentPlayerTurn][_skillType])
	{
		_state = ESkillButtonState::Used;
        UEffectManager::GetInstance().ClearAura(this); // Effect 해제

	}
	else if (_state == ESkillButtonState::Selected && UGameManager::GetInstance().m_currentSelectedSkill != _skillType)
	{
		_state = ESkillButtonState::Normal;
        UEffectManager::GetInstance().ClearAura(this); // Effect 해제

	}

    // Clear 시 원상복구
    bool bIsUsed = UGameManager::GetInstance().m_usedSkills[UGameManager::GetInstance().CurrentPlayerTurn][_skillType];
    if (bIsUsed)
    {
        _state = ESkillButtonState::Used;
    }
    else
    {
        if (_state == ESkillButtonState::Used)
        {
            _state = ESkillButtonState::Normal;
        }
        else if (_state == ESkillButtonState::Selected && UGameManager::GetInstance().m_currentSelectedSkill != _skillType)
        {
            _state = ESkillButtonState::Normal;
        }
    }
}

void USkillButton::Render(URenderer& renderer)
{
    if (!_isActive)
        return;

    switch (_state)
    {
    case ESkillButtonState::Normal:
        UUI::Render(renderer);
        break;
    case ESkillButtonState::Hovered:
        UUI::Render(renderer);
        // @Effect
        break;
    case ESkillButtonState::Selected:
        UUI::Render(renderer);
        
        // @Effect
        UEffectManager::GetInstance().DrawAura(
            this,  
            "Resources/item_aura.png",
            DirectX::XMFLOAT2(_x + _width * 0.5f, _y + _height * 0.5f),  // 버튼 중앙
            3.0f,
            DirectX::XMFLOAT2(1.0f, 1.0f),
            16
        );

        break;
    case ESkillButtonState::Used:
        _srv = UResourceManager::GetInstance().GetTexture(_usedTextureKey);
        RECT destRect = {
            (LONG)_x,
            (LONG)_y,
            (LONG)(_x + _width),
            (LONG)(_y + _height)
        };
        renderer.m_spriteBatch->Draw(_srv, destRect);
        break;
    }
}
