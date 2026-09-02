#pragma once

class URenderer;

enum class EFadeState { None, FadeIn, FadeOut };

class UFadeOverlay
{
public:
    EFadeState m_state = EFadeState::None;
    float m_duration = 1.0f;   // 페이드에 걸리는 시간(초)
    float m_timer = 0.0f;
    float m_alpha = 0.0f;      // 현재 오버레이 불투명도 (0~1)

    void StartFadeOut(float duration);
    void StartFadeIn(float duration);

    void Update(float deltaTime);
	void Render(URenderer& renderer);

    bool IsFading() const { return m_state != EFadeState::None; }
};
