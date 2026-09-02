#include "pch.h"
#include "UFadeOverlay.h"
#include "URenderer.h"
#include "UUI.h"
#include "UGameSetting.h"
#include "UResourceManager.h"

void UFadeOverlay::StartFadeOut(float duration)
{
    m_state = EFadeState::FadeOut; 
    m_duration = duration; 
    m_timer = 0.0f;
    m_alpha = 0.0f;
}

void UFadeOverlay::StartFadeIn(float duration)
{
    m_state = EFadeState::FadeIn;  
    m_duration = duration; 
    m_timer = 0.0f;
    m_alpha = 1.0f;
}

void UFadeOverlay::Update(float deltaTime)
{
    if (m_state == EFadeState::None) return;

    m_timer += deltaTime;

    float t = (((m_timer / m_duration) < (1.0f)) ? (m_timer / m_duration) : (1.0f));
    m_alpha = (m_state == EFadeState::FadeOut) ? t : (1.0f - t);

    if (t >= 1.0f)
        m_state = EFadeState::None;
}

void UFadeOverlay::Render(URenderer& renderer)
{
	if (m_alpha <= 0.0f) return;

    DirectX::XMVECTORF32 color = { 0.0f, 0.0f, 0.0f, m_alpha };

	float ScreenWidth = (float)UGameSetting::GetInstance().ScreendWidth;
	float ScreenHeight = (float)UGameSetting::GetInstance().ScreenHeight;
    ID3D11ShaderResourceView* srv = UResourceManager::GetInstance().GetTexture("Resources/black.png");

    renderer.m_spriteBatch->Draw(
        srv,
        RECT{ 0, 0, (LONG)ScreenWidth, (LONG)ScreenHeight },
        nullptr,
        color
    );
}