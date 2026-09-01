#pragma once
#include "pch.h"
#include "UBall.h"
#include "UResourceManager.h"

UBall::UBall()
{
    TotalNumBalls++;

    // 기본 값
    Elastic = 1.0f;
    GNumber = 1.0f;
    bEnableAngularVelocity = false;

    // 꽉찬 구의 관성 모멘트
    Inertia = 0.4f * Mass * Radius * Radius;

    // 임시로 테스트용 크기,위치 속도 설정
   // 랜덤 크기, 질량
    Radius = (0.09f + (rand() % 100) / 100.0f) * 100.0f;
    Mass = Radius * 10.0f;

    // 랜덤 위치
    Location.x = rand() % 1000 + 50.0f;
    Location.y = rand() % 1000 + 50.0f;
    Location.z = 0.0f;

    // 랜덤 속도
    Velocity.x = ((rand() % 200) - 100) * 10.0f;
    Velocity.y = ((rand() % 200) - 100) * 10.0f;
    Velocity.z = 0.0f;

    // 랜덤 Rotation
    Rotation.x = ((rand() % 200) - 100) * 100.0f;
    Rotation.y = ((rand() % 200) - 100) * 100.0f;
    Rotation.z = ((rand() % 200) - 100) * 100.0f;

    m_textureKey = "Resources/test.png";
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
}

void UBall::Update(float DeltaTime, float ScreendWidth, float ScreenHeight)
{    
    // 마찰력 계수 적용
    Location += Velocity * DeltaTime;
    if (Velocity.Length() > 1.0f) {
        FVector NormalizeFricVec = Velocity / Velocity.Length() * (-1.0f);
        if (Velocity.Length() <= 3000.0f * DeltaTime)
        {
            Velocity = FVector(0, 0, 0);
        }
        else Velocity += NormalizeFricVec * 3000.0f * DeltaTime;
    }
    if (bEnableAngularVelocity)
    {
        Rotation += AngularVelocity * DeltaTime;
    }

    // 각속도 서서히 멈추게
    AngularVelocity.x *= 0.99f;
    AngularVelocity.y *= 0.99f;
    AngularVelocity.z *= 0.99f;

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

void UBall::ApplyGravity(float DeltaTime)
{
    Velocity.y -= 9.8f * GNumber * DeltaTime;
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

void UBall::ApplySizeScaling(float scale)
{
    Radius *= scale;
}

void UBall::ApplyMassScaling(float scale)
{
    Mass *= scale;
}

void UBall::ApplySelfDestruct() 
{
    isSelfDestruct = true;
}

void UBall::ApplyEnableFreeze()
{
    bEnableFreeze = true;
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