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
    GNumber = 1.0f;
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

    int ScreendWidth = UGameSetting::GetInstance().ScreendWidth;
    int ScreenHeight = UGameSetting::GetInstance().ScreenHeight;

    // 벽에 부딪칠때 공 파괴 (임시)
    if ((Location.x < Radius) || (Location.x > ScreendWidth - Radius) || (Location.y < Radius) || (Location.y > ScreenHeight - Radius))
    {
        this->bIsDestroyed = true;
		USoundManager::GetInstance().PlaySound("out");
    }
}
    
void UBall::ApplyGravity(float DeltaTime)
{
    Velocity.y += 9800.0f * GNumber * DeltaTime;
}

void UBall::SetGNumber(float NewG)
{
    GNumber = NewG;
}

void UBall::SetElastic(float NewElastic)
{
    Elastic = NewElastic;
}

void UBall::SetTexture(ID3D11ShaderResourceView* srv)
{
	m_textureView = srv;
}

void UBall::ApplyReverseMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce)
{
    UBall* Other = dynamic_cast<UBall*>(OtherPrimitive);
    if (!Other)
    {
        return;
    }

    FVector Delta = Other->Location - Location; // 두 공사이 거리 벡터
    float DistSq = Delta.LengthSquared();       // 거리 제곱

    // 너무 가까히 말고 일정 거리 이내에서
    if (DistSq > 0.01f && DistSq < 250000.0f)
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

void UBall::ApplySelfFreeze()
{
    isFreezed = true;
}

void UBall::ApplyAirResistance(float DeltaTime, float AirResistance)
{
    float LinearSpeed = Velocity.Length();
    // 속도가 있을 때에 적용
    if (LinearSpeed > 0.0f)
    {
        // 항력 구하기
        FVector DragForce = Velocity * LinearSpeed * -AirResistance;

        // 드래그 가속도 구하기
        FVector DragAccelration = DragForce / Mass;

        // 속도에 적용
        Velocity += DragAccelration * DeltaTime;
    }
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
             break;

        case ESkillType::Freeze:
             bEnableFreeze = true;
             break;

        case ESkillType::Giant:
            TargetRadius = UGameSetting::GetInstance().BallBaseRadius * 1.5f;
            isSizeScaling = true;
			USoundManager::GetInstance().PlaySound("sizeup");
            break;

        case ESkillType::Heavier:
            TargetMass = UGameSetting::GetInstance().BallBaseRadius * 10.0f * 1.5f;
            isMassScaling = true;
			USoundManager::GetInstance().PlaySound("sizeup");
            break;

        case ESkillType::Repulse:
            isMagnetActivated = true;
            break;
        case ESkillType::WallCreate:
            bEnableWallCreate = true;
            break;
        case ESkillType::Shotgun:
            bEnableShotgun = true;
            ShotgunstartPos = Location;
            break;
        default:
            break;
    }
}

void UBall::RemoveAllSkill()
{
    isFreezed = false;
    bEnableFreeze = false;
    bEnableWallCreate = false;
    isSelfDestruct = false;
    TargetRadius = UGameSetting::GetInstance().BallBaseRadius;
    isSizeScaling = true;
    TargetMass = UGameSetting::GetInstance().BallBaseRadius * 10.0f;
    isMassScaling = true;
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
    float FricVal = 500.0f;
    if (isFreezed)
    {
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
                float currentMagnetForce = 300000.0f;
                for (auto* other : others)
                {
                    if (other != this)
                    {
                        if (other != nullptr)
                        {
                            ApplyReverseMagnetism(other, DeltaTime, currentMagnetForce);
                        }
                    }
                }
                AlreadyActiveMag = true;
				USoundManager::GetInstance().PlaySound("repulse");
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
        float currentMineForce = 500000.0f;
        for (auto* other : others)
        {
            if (other != this)
            {
                if (other != nullptr)
                {
                    ApplyReverseMagnetism(other, DeltaTime, currentMineForce);
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
