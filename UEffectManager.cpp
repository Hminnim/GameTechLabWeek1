#include "pch.h"
#include "UEffectManager.h"
#include "UResourceManager.h"
// #include "URenderer.h"

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
                            float duration, const DirectX::XMFLOAT2 scale, int frameCount, bool fadeOut, float rotation)
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
    effect.Rotation = rotation;
    effect.Duration = duration;
    effect.ElapsedTime = 0.0f;
    effect.FrameCount = frameCount;
    effect.FrameWidth = (int)desc.Width / effect.FrameCount;
    effect.FrameHeight = (int)desc.Height;
    effect.bFadeOut = fadeOut;

    char buffer[128];
    sprintf_s(buffer, "Texture Width: %d, Height: %d, FrameWidth: %d\n", (int)desc.Width, (int)desc.Height, effect.FrameWidth);
    OutputDebugStringA(buffer);

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

    // 화살표는 loop 반복
    if (m_hasArrow)
    {
        m_arrow.ElapsedTime += deltaTime;
        if (m_arrow.IsFinished())
            m_arrow.ElapsedTime = fmodf(m_arrow.ElapsedTime, m_arrow.Duration);
    }

    for (auto& pair : m_stayEffects) // pair: <key, FActiveEffect>
    {
        FActiveEffect& effect = pair.second;
        effect.ElapsedTime += deltaTime;
        if (effect.IsFinished())
            effect.ElapsedTime = fmodf(effect.ElapsedTime, effect.Duration);
    }
}

void UEffectManager::DrawArrow(const std::string& textureKey, const DirectX::XMFLOAT2& position,
                        float rotation, float loopDuration, const DirectX::XMFLOAT2 scale, int frameCount)
{   
    // 이미 화살표가 있다면 방향과 위치만 변경해주면 됨.
    // 당기는 거리에 비례하여 화살표가 늘어나도록 scale 추가함.
    if (m_hasArrow)
    {
        m_arrow.Position = position;
        m_arrow.Rotation = rotation;
        m_arrow.Scale = scale;
        
        return;
    }

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
    m_arrow.Texture = srv;
    m_arrow.Position = position;
    m_arrow.Scale = scale;
    m_arrow.Rotation = rotation;
    m_arrow.Duration = loopDuration;
    m_arrow.ElapsedTime = 0.0f;
    m_arrow.FrameCount = frameCount;
    m_arrow.FrameWidth = (int)desc.Width / m_arrow.FrameCount;
    m_arrow.FrameHeight = (int)desc.Height;
    m_arrow.bFadeOut = false;

    m_hasArrow = true;
}

void UEffectManager::ClearArrow()
{
    m_hasArrow = false;
}

void UEffectManager::DrawAura(void* key, const std::string& textureKey, const DirectX::XMFLOAT2& position,
                    float loopDuration, const DirectX::XMFLOAT2 scale, int frameCount)
{
    auto it = m_stayEffects.find(key); // <key, FActiveEffect>
    
    // 이미 얼음이 재생되고 있는 경우, 위치/크기 조절 가능 (사용하지는 않을듯)
    if (it != m_stayEffects.end())
    {
        it->second.Position = position;
        it->second.Scale = scale;
        return;
    }

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
    effect.Duration = loopDuration;
    effect.ElapsedTime = 0.0f;
    effect.FrameCount = frameCount;
    effect.FrameWidth = (int)desc.Width / effect.FrameCount;
    effect.FrameHeight = (int)desc.Height;
    effect.bFadeOut = false;

    m_stayEffects[key] = effect;
}

void UEffectManager::ClearAura(void* key)
{
    m_stayEffects.erase(key);
}

void UEffectManager::RenderAuras()
{
    if (m_stayEffects.empty())
        return;

    m_spriteBatch->Begin();

    
    // Freeze Effect
    for (const auto& pair : m_stayEffects)
    {
        const FActiveEffect effect = pair.second;

        char buf2[256];
        sprintf_s(buf2, "Aura Draw - Pos(%.1f,%.1f) Scale(%.2f,%.2f) FW=%d FH=%d FrameCount=%d\n",
        effect.Position.x, effect.Position.y,
        effect.Scale.x, effect.Scale.y,
        effect.FrameWidth, effect.FrameHeight, effect.FrameCount);
        OutputDebugStringA(buf2);

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
            DirectX::XMVECTORF32{ 1.0f, 1.0f, 1.0f, 0.5f },   // 알파(투명도)
            effect.Rotation,                                  // NO rotation
            origin,
            effect.Scale

        );
    }
    m_spriteBatch->End();

}

void UEffectManager::Render()
{
    if (m_activeEffects.empty() && !m_hasArrow)
        return;

    m_spriteBatch->Begin();
    // renderer.BeginSprite();

    char buf[64];
    sprintf_s(buf, "Render calling, m_hasArrow=%d\n", m_hasArrow);
    OutputDebugStringA(buf);


    // Normal Effect
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
            effect.Rotation,                       // NO rotation
            origin,
            effect.Scale

        );
        
    }

    // 화살표 Effect
    if (m_hasArrow)
    {
        float progress = m_arrow.GetProgress();

        int frameIdx = (int)(progress * m_arrow.FrameCount); // 진행률 * 총 프레임 개수 = 현재 프레임 인덱스
        
        // progress가 1.0이면, idx error 발생하므로 보정 필요
        if (frameIdx >= m_arrow.FrameCount)
            frameIdx = m_arrow.FrameCount - 1;
        
        RECT sourceRECT =
        {
            frameIdx * m_arrow.FrameWidth,       // left
            0,                                  // top
            (frameIdx + 1) * m_arrow.FrameWidth, // right
            m_arrow.FrameHeight                  // bottom
        };

        // Todo: FadeOut 처리
        
        // Effect 중심부터 Draw
        DirectX::XMFLOAT2 origin =
        {
            m_arrow.FrameWidth * 0.5f,
            m_arrow.FrameHeight * 0.5f
        };

        m_spriteBatch->Draw(
            m_arrow.Texture,
            m_arrow.Position,
            &sourceRECT,
            DirectX::Colors::White,
            m_arrow.Rotation,                       // NO rotation
            origin,
            m_arrow.Scale

        );
    }

    m_spriteBatch->End();
}