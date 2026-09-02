#pragma once
#include "pch.h"
#include "UBall.h"
#include "UResourceManager.h"
#include "UGameSetting.h"
#include "UEffectManager.h"

CollisionManager CollisionMan;

UBall::UBall(const std::string& meshKey, const EPlayer owner, const FVector startLocation)
{
    TotalNumBalls++;

    Owner = owner;

    // 기본 값
    Elastic = 1.0f;
    GNumber = 1.0f;
    bEnableAngularVelocity = false;

    // 꽉찬 구의 관성 모멘트
    Inertia = 0.4f * Mass * Radius * Radius;

    // 반지름, 반지름에 비례한 질량
    Radius = 50.0f;
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

    
    float FricVal=500.0f;
    if (isFreezed)
    {
        FricVal = 200000.0f;
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
                            ApplyReverseMagnetism(other, DeltaTime, currentMagnetForce);
                        }
                    }
                }
                AlreadyActiveMag = true;
            }
        }
        else Velocity += NormalizeFricVec * FricVal * DeltaTime;
    }
    if (bEnableAngularVelocity)
    {
        Rotation += AngularVelocity * DeltaTime;
    }

    int ScreendWidth = UGameSetting::GetInstance().ScreendWidth;
    int ScreenHeight = UGameSetting::GetInstance().ScreenHeight;

    // 벽에 부딫칠때 감속 및 방향 전환
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

    //충돌 처리
    for (auto* other : others)
    {
        if (other != this && !other->bIsDestroyed)
        {
            UBall* otherBall = dynamic_cast<UBall*>(other);
            if (otherBall != nullptr && !otherBall->bIsDestroyed)
            {
                CollisionMan.ResolveCollision(this, otherBall);

            }
        }
    }

    // 자폭 적용된 공 삭제 전 주변에 척력 적용
    if (this->bIsDestroyed && this->isSelfDestruct)
    {
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

        //for (auto it = others.begin();it != others.end();it++)
        //{
        //    if (*it == this)
        //    {
        //        others.erase(it);
        //        delete this;
        //        return;
        //    }
        //}
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

void UBall::ApplySkill(USkillType skill)
{
    switch (skill)
    {
        case USkillType::Mine:
             isSelfDestruct = true;
             break;

        case USkillType::Freeze:
             bEnableFreeze = true;
             break;

        case USkillType::SizeScaling:
            TargetRadius = Radius * 1.5;
            isSizeScaling = true;
             break;

        case USkillType::MassScaling:
            TargetMass = Mass * 1.5;
            isMassScaling = true;
            break;

        case USkillType::ReverseMagnet:
            isMagnetActivated = true;
            break;
    }
}