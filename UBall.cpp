#pragma once
#include "pch.h"
#include "UBall.h"

UBall::UBall()
{
    TotalNumBalls++;

    // 기본 값
    Elastic = 1.0f;
    GNumber = 1.0f;
    bEnableAngularVelocity = false;

    // 랜덤 크기, 질량
    Radius = (0.09f + (rand() % 100) / 100.0f) * 0.1f;
    Mass = Radius * 10.0f;

    // 꽉찬 구의 관성 모멘트
    Inertia = 0.4f * Mass * Radius * Radius;

    // 랜덤 위치
    Location.x = ((rand() % 200) - 100) * 0.01f;
    Location.y = ((rand() % 200) - 100) * 0.01f;
    Location.z = 0.0f;

    // 랜덤 속도
    Velocity.x = ((rand() % 200) - 100) * 0.005f;
    Velocity.y = ((rand() % 200) - 100) * 0.005f;
    Velocity.z = 0.0f;

    // 랜덤 Rotation
    Rotation.x = ((rand() % 200) - 100) * 0.01f;
    Rotation.y = ((rand() % 200) - 100) * 0.01f;
    Rotation.z = ((rand() % 200) - 100) * 0.01f;
}

UBall::~UBall()
{
    TotalNumBalls--;
}

void UBall::Render(URenderer& Renderer)
{
    Renderer.UpdateConstant(Location, Radius, Rotation);
}

void UBall::Update(float DeltaTime)
{
    Location += Velocity * DeltaTime;

    if (bEnableAngularVelocity)
    {
        Rotation += AngularVelocity * DeltaTime;
    }

    // 각속도 서서히 멈추게
    AngularVelocity.x *= 0.99f;
    AngularVelocity.y *= 0.99f;
    AngularVelocity.z *= 0.99f;

    if (Location.x < -1.0f + Radius)
    {
        Velocity.x *= -1.0f * Elastic;
        Location.x = -1.0f + Radius;
    }
    if (Location.x > 1.0f - Radius)
    {
        Velocity.x *= -1.0f * Elastic;
        Location.x = 1.0f - Radius;
    }
    if (Location.y < -1.0f + Radius)
    {
        Velocity.y *= -1.0f * Elastic;
        Location.y = -1.0f + Radius;
    }
    if (Location.y > 1.0f - Radius)
    {
        Velocity.y *= -1.0f * Elastic;
        Location.y = 1.0f - Radius;
    }
}

void UBall::ResolveCollision(UPrimitive* OtherPrimitive)
{
    // Casting UBall class
    UBall* Other = dynamic_cast<UBall*>(OtherPrimitive);
    if (!Other)
    {
        return;
    }

    // 충돌 감지
    FVector Delta = Location - Other->Location;
    float DistSq = Delta.LengthSquared();
    float SumRadius = Radius + Other->Radius;

    // 충돌 상태
    if (DistSq < SumRadius * SumRadius)
    {
        // 거리 계산
        float Dist = (float)sqrt(DistSq);

        // 0 나누기 방지
        if (Dist == 0.0f)
        {
            return;
        }

        // 충돌 법선 벡터
        FVector NormalVector = Delta / Dist;

        // 겹친 상태 떼어 놓기
        float Overlap = SumRadius - Dist; // 겹친 길이

        // 질량에 따라 비율
        float TotalMass = Mass + Other->Mass;
        float M1Ratio = Other->Mass / TotalMass;
        float M2Ratio = Mass / TotalMass;

        // 겹친 만큼 서로 반대 방향으로 밀기
        Location += NormalVector * Overlap * M1Ratio;
        Other->Location -= NormalVector * Overlap * M2Ratio;

        // Impulse 구하기
        FVector vRel = Velocity - Other->Velocity;  // 상대 속도
        float VelAlongNormal = vRel.Dot(NormalVector);    // 법선 방향(충돌 축)의 속도 성분

        // 두 공이 충돌 상태가 아님(멀어지고 있음)
        if (VelAlongNormal > 0.0f)
        {
            return;
        }

        // 선형 속도
        // 충격량 계산
        // j = -(1 + e) * (vRel· n) / (1 / m1 + 1 / m2)
        float jNormal = -(1.0f + Elastic) * VelAlongNormal;
        jNormal /= (1.0f / Mass + 1.0f / Other->Mass);

        // 충격량 벡터 생성
        FVector NormalImpulse = NormalVector * jNormal;

        // 운동량 변화 적용 ( a = F / m)
        Velocity += NormalImpulse / Mass;
        Other->Velocity -= NormalImpulse / Other->Mass; // 작용 반장욕

        // 각속도 처리
        if (bEnableAngularVelocity)
        {
            // 접선 벡터(Tangent) 계산-> 충돌 표면에 따라 미끄러지는 방향으로
            // Tangent = RelativeVelocity - NormalComponent
            FVector TangentVector = vRel - NormalVector * NormalVector.Dot(NormalVector);
            float TangentLen = TangentVector.Length();

            // 작은 값은 무시
            if (TangentLen > 0.001f)
            {
                // 단위 벡터로 정규화
                TangentVector = TangentVector / TangentLen;

                // 마찰 충격량 계산                                  
                float Friction = 0.5f; // 마찰 계수는 임의로 0.5f로 설정
                FVector FrictionImpulse = TangentVector * -1.0f * jNormal * Friction; // 마찰력은 운동 반대 방향

                // 충돌 지점까지의 반지름 벡터
                FVector R1 = NormalVector * Radius;
                FVector R2 = NormalVector * -Other->Radius;

                // 각속도 적용
                AngularVelocity += R1.Cross(FrictionImpulse) / Inertia;
                Other->AngularVelocity += R2.Cross(FrictionImpulse * -1.0f) / Other->Inertia; // 작용 반작용으로 반대 방향
            }
        }

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
    if (DistSq > 0.001f && DistSq < 3.0f)
    {
        float Dist = (float)sqrt(DistSq);

        FVector Normal = Delta / Dist; // 법선 벡터

        // 거리에 비례해서 강한 힘
        float Force = MagneticForce / DistSq;

        // 힘의 방향 벡터
        FVector ForceVector = Normal * Force;

        // 질량이 가벼울 수록 빠르게 접근 F = ma
        Velocity -= (ForceVector / Mass);
        Other->Velocity += (ForceVector / Other->Mass); // 작용 반작용
    }
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