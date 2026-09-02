#pragma once
#include "pch.h"
#include "UButton.h"
#include "URenderer.h"
#include "UResourceManager.h"
#include "UGameManager.h"
#include "UGameSetting.h"
#include "USoundManager.h"

bool UButton::HitTest(float mouseX, float mouseY) const
{
	if (!_isActive)
		return false;
    return (mouseX >= _x && mouseX <= _x + _width) &&
        (mouseY >= _y && mouseY <= _y + _height);
}

void UButton::OnClick()
{
	USoundManager::GetInstance().PlaySound("select_menu");
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
	if (_state == ESkillButtonState::Normal)
	{
		_state = ESkillButtonState::Selected;
		UGameManager::GetInstance().m_currentSelectedSkill = _skillType;
		USoundManager::GetInstance().PlaySound("select_item");
	}
}

void USkillButton::Update(float deltaTime)
{
	if (UGameManager::GetInstance().m_usedSkills[UGameManager::GetInstance().CurrentPlayerTurn][_skillType])
	{
		_state = ESkillButtonState::Used;
	}
	else if (_state == ESkillButtonState::Selected && UGameManager::GetInstance().m_currentSelectedSkill != _skillType)
	{
		_state = ESkillButtonState::Normal;
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
