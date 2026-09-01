#include "pch.h"
#include "UEffectManager.h"
#include "UResourceManager.h"

void UEffectManager::Init(ID3D11DeviceContext* context)
{
    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);
}

void UEffectManager::Release()
{
    m_activeEffects.clear();
    m_spriteBatch.reset();   
}

void UEffectManager::PlayEffect(const std::string& textureKey, const DirectX::XMFLOAT2& position, 
                            float duration, float scale, int frameCount, bool fadeOut)
{
    ID3D11ShaderResourceView* srv = UResourceManager::GetInstance().GetTexture(textureKey);
    if (!srv) {
        OutputDebugStringA("Texture Load Failed!\n");
        assert(false);
    }
    
    // SRV 가로 pixel size 가져오기
    ID3D11Resource* resource = nullptr;
    srv->GetResource(&resource);

    ID3D11Texture2D* texture2D = nullptr;
    resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture2D);

    D3D11_TEXTURE2D_DESC desc = {};
    texture2D->GetDesc(&desc); // desc.Width, desc.Height에 가로/세로 pixel size 저장

    resource->Release();
    texture2D->Release();

    // Effect Setting
    FActiveEffect effect;
    effect.Texture = srv;
    effect.Position = position;
    effect.Scale = scale;
    effect.Duration = duration;
    effect.ElapsedTime = 0.0f;
    effect.FrameCount = frameCount;
    effect.FrameWidth = (int)desc.Width / effect.FrameCount;
    effect.FrameHeight = (int)desc.Height;
    effect.bFadeOut = fadeOut;

    m_activeEffects.push_back(effect);
}

void UEffectManager::Update(float deltaTime)
{
    for (auto& effect : m_activeEffects)
        effect.ElapsedTime += deltaTime;

    m_activeEffects.erase(
        // IsFinished() -> ture 면 배열 뒤로 밀어낸 후. 첫번째 False Effect idx 가리킴.
        std::remove_if(m_activeEffects.begin(), m_activeEffects.end(), [](const FActiveEffect& e) { return e.IsFinished(); }),
        // 그 뒤 Effect도 False 이므로 모두 배열에서 삭제
        m_activeEffects.end()
    );
}
void UEffectManager::Render()
{
    if (m_activeEffects.empty())
        return;

    m_spriteBatch->Begin();

    for (const auto& effect : m_activeEffects)
    {
        float progress = effect.GetProgress();

        int frameIdx = (int)(progress * effect.FrameCount); // 진행률 * 총 프레임 개수 = 현재 프레임 인덱스
        
        // progress가 1.0이면, idx error 발생하므로 보정 필요
        if (frameIdx >= effect.FrameCount)
            frameIdx = effect.FrameCount - 1;

        
        RECT sourceRECT =
        {
            frameIdx * effect.FrameWidth,       // left
            0,                                  // top
            (frameIdx + 1) * effect.FrameWidth, // right
            effect.FrameHeight                  // bottom
        };

        // Todo: FadeOut 처리
        
        // Effect 중심부터 Draw
        DirectX::XMFLOAT2 origin =
        {
            effect.FrameWidth * 0.5f,
            effect.FrameHeight * 0.5f
        };

        m_spriteBatch->Draw(
            effect.Texture,
            effect.Position,
            &sourceRECT,
            DirectX::Colors::White,
            0.0f,                       // NO rotation
            origin,
            effect.Scale
        );


        m_spriteBatch->End();
    }
}