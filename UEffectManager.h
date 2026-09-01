#pragma once
#include <d3d11.h>
#include <SpriteBatch.h>
#include <vector>
#include <memory>
#include <string>
#include "MathTypes.h"


// class URenderer;

// 이펙트 구조(Texture, Position, Scale, Duration, ElapsedTime, FrameCount, FrameWidth, FrameHeight, bFadeOut)
// 스프라이트시트 애니메이션 지원 (세로는 미지원)
struct FActiveEffect
{
    ID3D11ShaderResourceView* Texture = nullptr;
    DirectX::XMFLOAT2 Position = { 0.0f, 0.0f };
    float Scale = 1.0f;

    // 총 재생 시간(s)
    float Duration = 1.0f;

    // 경과 시간(s)
    float ElapsedTime = 0.0f;

    // 애니메이션 프레임
    int FrameCount = 1; // 프레임 개수 (1이면 정적)
    int FrameWidth = 0; // 프레임 가로 크기 (pixel)
    int FrameHeight = 0; // 프레임 세로 크기 (pixel)

    // Fade out 여부
    bool bFadeOut = true;

    // (경과시간 >= 재생 시간) -> true
    bool IsFinished() const { return ElapsedTime >= Duration; }

    // 이펙트 실행률
    float GetProgress() const { return Duration > 0.0f ? ElapsedTime / Duration : 1.0f; }
};

class UEffectManager
{
    public:
        static UEffectManager& GetInstance()
        {
            static UEffectManager instance;
            return instance;
        }

        UEffectManager(const UEffectManager&) = delete;
        UEffectManager& operator=(const UEffectManager&) = delete;

        void Init(ID3D11DeviceContext* context);
        void Release();

        // 이펙트 재생 (텍스처키, 이펙트 위치, 재생시간, 비율, 프레임개수, 페이드아웃 여부)
        void PlayEffect(const std::string& textureKey, const DirectX::XMFLOAT2& position, 
                        float duration, float scale = 1.0f, int frameCount = 1, bool fadeOut = false);


        void Update(float deltaTime);
        void Render();

    private:
        UEffectManager() = default;
        ~UEffectManager() = default;

        std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
        std::vector<FActiveEffect> m_activeEffects;

};