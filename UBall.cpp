#pragma once
#include "pch.h"
#include "UBall.h"
#include "UWall.h"
#include "UResourceManager.h"
#include "UGameSetting.h"
#include "UEffectManager.h"
#include "UScene.h"
#include "USceneManager.h"
#include "USoundManager.h"

CollisionManager CollisionMan;
FVector lastspawnpos;
FVector ShotgunstartPos;

UBall::UBall(const std::string& meshKey, const EPlayer owner, const FVector startLocation)
{
    TotalNumBalls++;
    
    Owner = owner;

    // 기본 값
    Elastic = 1.0f;
    bIsDestroyed = false;
    bEnableAngularVelocity = false;

    // 꽉찬 구의 관성 모멘트
    Inertia = 0.4f * Mass * Radius * Radius;

    // 반지름, 반지름에 비례한 질량
    Radius = UGameSetting::GetInstance().BallBaseRadius;
    Mass = Radius * 10.0f;

    // 공 시작 위치
    Location = startLocation;

    // 초기에는 멈춤
    Velocity.x = 0.0f;
    Velocity.y = 0.0f;
    Velocity.z = 0.0f;

    //
    Rotation.x = 1.57f;
    Rotation.y = 0;
    Rotation.z = 0;

    m_vertexBuffer = UResourceManager::GetInstance().GetVertexBuffer(meshKey);
	m_numVertices = UResourceManager::GetInstance().GetNumVertices(meshKey);

    m_textureKey = (Owner == EPlayer::Red) ? "Resources/red.png" : "Resources/blue.png";
	m_textureView = UResourceManager::GetInstance().GetTexture(m_textureKey);
}

UBall::~UBall()
{
    TotalNumBalls--;
    UEffectManager::GetInstance().ClearAura(this);
    UEffectManager::GetInstance().ClearAura(&_skillAuraKey);
}

void UBall::Render(URenderer& Renderer)
{
    Renderer.UpdateConstant(Location, Radius, Rotation);
	Renderer.BindTexture(0, m_textureView);
    Renderer.RenderPrimitive(m_vertexBuffer, m_numVertices);
}

void UBall::Update(float DeltaTime, std::vector<UPrimitive*>& others)
{
    UBall::SizeMassScaling(DeltaTime);

    UBall::WallCollision();

    UBall::FrictionFloor(DeltaTime, others);

    UBall::CollisionManage(DeltaTime, others);

    UBall::ReverseMagnetWhenMine(DeltaTime, others);

    float ScreendWidth = (float)UGameSetting::GetInstance().ScreendWidth;
    float ScreenHeight = (float)UGameSetting::GetInstance().ScreenHeight;
    
    float MapMarginX = UGameSetting::GetInstance().MapMarginX;
    float MapMarginY = UGameSetting::GetInstance().MapMarginY;

    // 벽에 부딪칠때 공 파괴 (임시)
  //  if ((Location.x < Radius) || (Location.x > ScreendWidth - Radius) || (Location.y < Radius) || (Location.y > ScreenHeight - Radius))
  //  {
  //      this->bIsDestroyed = true;
		//USoundManager::GetInstance().PlaySound("out");
  //  }

    if ((Location.x < MapMarginX - Radius) 
        || (Location.x > ScreendWidth - MapMarginX + Radius) 
        || (Location.y > ScreenHeight - MapMarginY + Radius) 
        || (Location.y < MapMarginY - Radius))
    {
        
        if (bEnableReturn)
        {
            Location = Returnpos;
            Velocity = FVector(0, 0, 0);
            this->bIsDestroyed = false;
        }
        else 
        {
            this->bIsDestroyed = true;
            USoundManager::GetInstance().PlaySound("out");
        }
    }

    // 이동하는 중에도 Effect 적용
    if ( bEnableFreeze || isSelfDestruct || isMagnetActivated || isGiantActivated || isHeavierActivated
        || bEnableWallCreate || bEnableGhost || bEnableMagnet ||bEnableReturn || bEnableShotgun )
    {

        // 각 skill 별 texture-key
        std::string Texture = ""; // texturekey

        if (bEnableFreeze)              Texture = "Resources/blue_aura.png";
        else if (isSelfDestruct)        Texture = "Resources/red_aura.png";
        else if (isMagnetActivated)     Texture = "Resources/purple_aura.png";
        else if (isGiantActivated)      Texture = "Resources/green_aura.png";
        else if (isHeavierActivated)    Texture = "Resources/yellow_aura.png";
        else if (bEnableWallCreate)     Texture = "Resources/wall_aura.png";
        else if (bEnableGhost)          Texture = "Resources/ghost_aura.png";
        else if (bEnableMagnet)         Texture = "Resources/magnet_aura.png";
        else if (bEnableReturn)         Texture = "Resources/return_aura.png";
        else if (bEnableShotgun)        Texture = "Resources/shotgun_aura.png";

        UEffectManager::GetInstance().DrawAura(
            &_skillAuraKey,
            Texture,
            DirectX::XMFLOAT2(Location.x, Location.y),
            1.0f,
            DirectX::XMFLOAT2(0.6f, 0.6f),
            16
        );
    }
}
void UBall::SetElastic(float NewElastic)
{
    Elastic = NewElastic;
}

void UBall::SetTexture(ID3D11ShaderResourceView* srv)
{
	m_textureView = srv;
}

void UBall::ApplyReverseMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce, float MaxDist)
{
    UBall* Other = dynamic_cast<UBall*>(OtherPrimitive);
    if (!Other)
    {
        return;
    }

    FVector Delta = Other->Location - Location; // 두 공사이 거리 벡터
    float DistSq = Delta.LengthSquared();       // 거리 제곱

    // 너무 가까히 말고 일정 거리 이내에서
    if (DistSq > 0.01f && DistSq < MaxDist)
    {
        float Dist = (float)sqrt(DistSq);

        FVector Normal = Delta / Dist; // 법선 벡터

        // 거리에 비례해서 강한 힘
        float Force = MagneticForce;

        // 힘의 방향 벡터
        FVector ForceVector = Normal * Force;

        // 질량이 가벼울 수록 빠르게 접근 F = ma
        Other->Velocity += (ForceVector / Other->Mass); // 작용 반작용
    }
}

void UBall::ApplyMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce, float MaxDist)
{
    UBall* Other = dynamic_cast<UBall*>(OtherPrimitive);
    if (!Other)
    {
        return;
    }

    FVector Delta = Other->Location - Location; // 두 공사이 거리 벡터
    float DistSq = Delta.LengthSquared();       // 거리 제곱

    // 너무 가까히 말고 일정 거리 이내에서
    if (DistSq > 0.01f && DistSq < MaxDist)
    {
        float Dist = (float)sqrt(DistSq);

        FVector Normal = Delta / Dist; // 법선 벡터

        // 거리에 비례해서 강한 힘
        float Force = MagneticForce;

        // 힘의 방향 벡터
        FVector ForceVector = Normal * Force;

        // 질량이 가벼울 수록 빠르게 접근 F = ma
        Other->Velocity -= (ForceVector / Other->Mass); // 작용 반작용
    }
}

void UBall::ApplySelfFreeze()
{
    isFreezed = true;
}

void UBall::SetEnableAngularMomentum(bool bEnable)
{
    bEnableAngularVelocity = bEnable;
}

void UBall::ApplySkill(ESkillType skill)
{
    switch (skill)
    {
        case ESkillType::Mine:
             isSelfDestruct = true;

            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);

            // Self-destruct Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/red_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );

             break;

        case ESkillType::Freeze:
             bEnableFreeze = true;

            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);

            // Freeze Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/blue_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );

             break;

        case ESkillType::Giant:
            TargetRadius = UGameSetting::GetInstance().BallBaseRadius * 1.5f;
            isSizeScaling = true;
            isGiantActivated = true;
			USoundManager::GetInstance().PlaySound("sizeup");
            
            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);

            // Giant Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/green_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );
            break;

        case ESkillType::Heavier:
            TargetMass = UGameSetting::GetInstance().BallBaseRadius * 10.0f * 1.5f;
            isMassScaling = true;
            isHeavierActivated = true;
			USoundManager::GetInstance().PlaySound("sizeup");

            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);

            // Havier Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/yellow_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );
            break;

        case ESkillType::Repulse:
            isMagnetActivated = true;

            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);
            
            // Repulse Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/purple_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );

            break;

        case ESkillType::WallCreate:
            bEnableWallCreate = true;

            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);
                        
            // Wall Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/wall_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );

            break;

        case ESkillType::Shotgun:
            bEnableShotgun = true;
            ShotgunstartPos = Location;

            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);
            break;

            // shotgun Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/shotgun_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );


        case ESkillType::Ghost:
            bEnableGhost = true;

            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);
            break;

            // ghost Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/ghost_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );
        
        case ESkillType::Magnet:
            bEnableMagnet = true;

            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);
            break;

            // magnet Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/magnet_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );

        case ESkillType::Return:
            bEnableReturn = true;
            Returnpos = Location;

            // 기존 effect 해제
            UEffectManager::GetInstance().ClearAura(this);
            break;

            // return Selected Ball Effect
            UEffectManager::GetInstance().DrawAura(
                &_skillAuraKey,
                "Resources/return_aura.png",
                DirectX::XMFLOAT2(this->Location.x, this->Location.y),
                1.0f,
                // 1.0f,
                DirectX::XMFLOAT2(0.5f, 0.5f),
                16
            );

        default:
            break;
    }
}

void UBall::RemoveAllSkill()
{
    isFreezed = false;
    bEnableFreeze = false;
    bEnableWallCreate = false;
    bEnableGhost = false;
    bEnableMagnet = false;
    bEnableReturn = false;
    isSelfDestruct = false;
    TargetRadius = UGameSetting::GetInstance().BallBaseRadius;
    isSizeScaling = true;
    TargetMass = UGameSetting::GetInstance().BallBaseRadius * 10.0f;
    isMagnetActivated = false;
    isMassScaling = true;
    bEnableShotgun = false;
    isGiantActivated = false;
    isHeavierActivated = false;
    

    UEffectManager::GetInstance().ClearAura(&_skillAuraKey);

}

void UBall::WallCollision()
{
    int ScreendWidth = UGameSetting::GetInstance().ScreendWidth;
    int ScreenHeight = UGameSetting::GetInstance().ScreenHeight;
    
    if (Location.x < Radius)
    {
        Velocity.x *= -0.9f * Elastic;
        Location.x = Radius;
    }
    if (Location.x > ScreendWidth - Radius)
    {
        Velocity.x *= -0.9f * Elastic;
        Location.x = ScreendWidth - Radius;
    }
    if (Location.y < Radius)
    {
        Velocity.y *= -0.9f * Elastic;
        Location.y = Radius;
    }
    if (Location.y > ScreenHeight - Radius)
    {
        Velocity.y *= -0.9f * Elastic;
        Location.y = ScreenHeight - Radius;
    }
}

void UBall::SizeMassScaling(float DeltaTime)
{
    if (isSizeScaling)
    {
        if (Radius < TargetRadius) {
            Radius += 30.0f * DeltaTime;
            if (Radius > TargetRadius)
            {
                Radius = TargetRadius;
                isSizeScaling = false;
            }
        }
        if (Radius > TargetRadius) {
            Radius -= 30.0f * DeltaTime;
            if (Radius < TargetRadius)
            {
                Radius = TargetRadius;
                isSizeScaling = false;
            }
        }
    }

    if (isMassScaling)
    {
        if (Mass < TargetMass) {
            Mass += 30.0f * DeltaTime;
            if (Mass > TargetMass)
            {
                Mass = TargetMass;
                isMassScaling = false;
            }
        }
        if (Mass > TargetMass) {
            Mass -= 30.0f * DeltaTime;
            if (Mass < TargetMass)
            {
                Mass = TargetMass;
                isMassScaling = false;
            }
        }
    }
}

void UBall::FrictionFloor(float DeltaTime, std::vector<UPrimitive*>& others)
{
    float FricVal = 700.0f;
    if (isFreezed)
    {
        UEffectManager::GetInstance().ClearAura(&_skillAuraKey);   

        // Freeze Effect
        UEffectManager::GetInstance().DrawAura(
            this,
            "Resources/new_freeze.png",
            DirectX::XMFLOAT2(this->Location.x, this->Location.y),
            1.0f,
            // 1.0f,
            DirectX::XMFLOAT2(0.5f, 0.5f),
            5
        );

        FricVal = 200000.0f;
    }
     else if (!isFreezed && bWasFreezed)
    {  
        // Unfreeze Effect
        UEffectManager::GetInstance().ClearAura(this);   // Freeze Effect 해제

        UEffectManager::GetInstance().PlayEffect(
            "Resources/unfreeze.png",
            DirectX::XMFLOAT2(this->Location.x, this->Location.y),
            1.0f,
            DirectX::XMFLOAT2(0.5f, 0.5f),
            8
        );
    }

    bWasFreezed = isFreezed;

    if (bEnableWallCreate && Velocity.Length()>50.0f && currentWallCount<MaxWallCount)
    {
        float dx = (Location.x - lastspawnpos.x);
        float dy = (Location.y - lastspawnpos.y);
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist >= 150.0f) {
            FVector dir = Velocity / Velocity.Length();
            float offset = Radius + 40.0f + 10.0f;
            FVector spawnpos = Location - (dir * offset);

            float angle = atan2f(Velocity.y, Velocity.x);

            UScene* currentScene = USceneManager::GetInstance().GetCurrentScene();
            currentScene->AddPrimitive(new UWall("square", FVector(spawnpos.x, spawnpos.y, 0.5f), 75.0f, this->Owner, -angle));
            
            lastspawnpos = spawnpos;
            currentWallCount++;
        }
    }


    if (bEnableShotgun)
    {       
        float dx = (Location.x - ShotgunstartPos.x);
        float dy = (Location.y - ShotgunstartPos.y);
        float dist = sqrtf(dx * dx + dy * dy);
        
        if (dist >= 150.0f) {

            // 발사 시 이펙트 해제
            UEffectManager::GetInstance().ClearAura(&_skillAuraKey);

            float angle = atan2f(Velocity.y, Velocity.x);
            FVector leftdir = FVector(cosf(angle - 0.20f), sin(angle - 0.20f), 0.0f);
            UBall* leftBall = new UBall("sphere", this->Owner, Location);
            leftBall->Velocity = leftdir * Velocity.Length();
            leftBall->isShotgunbullet = true;
            FVector rightdir = FVector(cosf(angle + 0.20f), sin(angle + 0.20f), 0.0f);
            UBall* rightBall = new UBall("sphere", this->Owner, Location);
            rightBall->Velocity = rightdir * Velocity.Length();
            rightBall->isShotgunbullet = true;
            UScene* currentScene = USceneManager::GetInstance().GetCurrentScene();
            currentScene->AddPrimitive(leftBall);
            currentScene->AddPrimitive(rightBall);
            bEnableShotgun = false;
        }
    }
    

    //속도에 따른 위치 이동
    Location += Velocity * DeltaTime;

    // 마찰력 계수 적용
    if (Velocity.Length() > 0.0f) {
        FVector NormalizeFricVec = Velocity / Velocity.Length() * (-1.0f);
        if (Velocity.Length() <= FricVal * DeltaTime)
        {
            Velocity = FVector(0, 0, 0);
            // 척력 발생시 주위 밀어냄
            if (isMagnetActivated && !AlreadyActiveMag)
            {
                float currentMagnetForce = 100000.0f;
                for (auto* other : others)
                {
                    if (other != this)
                    {
                        if (other != nullptr)
                        {
                            ApplyReverseMagnetism(other, DeltaTime, currentMagnetForce, 40000.0f);
                        }
                    }
                }
                AlreadyActiveMag = true;
				USoundManager::GetInstance().PlaySound("repulse");
                UEffectManager::GetInstance().ClearAura(&_skillAuraKey);
            }
            if (bEnableMagnet && !AlreadyActiveMagnetism)
            {
                float currentMagnetForce = 100000.0f;
                for (auto* other : others)
                {
                    if (other != this)
                    {
                        if (other != nullptr)
                        {
                            ApplyMagnetism(other, DeltaTime, currentMagnetForce, 40000.0f);
                        }
                    }
                }
                AlreadyActiveMagnetism = true;
            }
        }
        else Velocity += NormalizeFricVec * FricVal * DeltaTime;
    }
    if (bEnableAngularVelocity)
    {
        Rotation += AngularVelocity * DeltaTime;
    }
}

void UBall::ReverseMagnetWhenMine(float DeltaTime, std::vector<UPrimitive*>& others)
{
    // 자폭시 척력 적용
    if (this->bIsDestroyed && this->isSelfDestruct)
    {
        // 기존 effect 해제
        UEffectManager::GetInstance().ClearAura(this);
        UEffectManager::GetInstance().ClearAura(&_skillAuraKey);   

        UEffectManager::GetInstance().PlayEffect(
            "Resources/self-destruct.png",
            DirectX::XMFLOAT2(Location.x, Location.y),
            1.25f,                         // 재생 시간 (1.25초)
            DirectX::XMFLOAT2(2.0f, 2.0f), // 크기 배율 (2배)
            9,                             // 총 9프레임 스프라이트 시트
            false,
            0.0f
        );
        USoundManager::GetInstance().PlaySound("mine");
        float currentMineForce = 300000.0f;
        for (auto* other : others)
        {
            if (other != this)
            {
                if (other != nullptr)
                {
                    ApplyReverseMagnetism(other, DeltaTime, currentMineForce, 16000.0f);
                }
            }
        }
    }
}

void UBall::CollisionManage(float DeltaTime, std::vector<UPrimitive*>& others)
{
    for (auto* other : others)
    {
        if (other != this && !other->bIsDestroyed)
        {
            UWall* otherWall = dynamic_cast<UWall*>(other);
            if (otherWall != nullptr)
            {
                CollisionMan.ResolveWallCollision(this, otherWall);
            }
            UBall* otherBall = dynamic_cast<UBall*>(other);
            if (otherBall != nullptr)
            {
                CollisionMan.ResolveCollision(this, otherBall);
            }
        }
    }
}
