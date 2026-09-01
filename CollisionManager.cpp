#include "pch.h"
#include "CollisionManager.h"
#include "UBall.h"
#include "UEffectManager.h"

CollisionManager::CollisionManager()
{
}

CollisionManager::~CollisionManager()
{
}

bool CollisionManager::DetectCollision(UPrimitive* TargetPrimitive, UPrimitive* OtherPrimitive)
{
    UBall* TargetBall = dynamic_cast<UBall*>(TargetPrimitive);
    UBall* OtherBall = dynamic_cast<UBall*>(OtherPrimitive);

    // 충돌 감지
    FVector Delta = TargetPrimitive->Location - OtherPrimitive->Location;
    float DistSq = Delta.LengthSquared();
    float SumRadius = TargetPrimitive->Radius + OtherPrimitive->Radius;

    if (DistSq < SumRadius * SumRadius) return true;
    else return false;
}

void CollisionManager::ResolveCollision(UPrimitive* TargetPrimitive, UPrimitive* OtherPrimitive)
{
    UBall* TargetBall = dynamic_cast<UBall*>(TargetPrimitive);
    UBall* OtherBall = dynamic_cast<UBall*>(OtherPrimitive);
    
    FVector Delta = TargetPrimitive->Location - OtherPrimitive->Location;
    float DistSq = Delta.LengthSquared();
    float SumRadius = TargetPrimitive->Radius + OtherPrimitive->Radius;

    // 충돌 상태
    if (DetectCollision(TargetPrimitive,OtherPrimitive))
    {
        if (TargetBall->isSelfDestruct)
        {
            TargetPrimitive->isDestroyed = true;
            return;
        }
        else if (OtherBall->isSelfDestruct)
        {
            OtherPrimitive->isDestroyed = true;
            return;
        }
        
        if (TargetBall->bEnableFreeze)
        {
            OtherBall->isFreezed = true;
        }
        if (OtherBall->bEnableFreeze)
        {
            TargetBall->isFreezed = true;
        }

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
        float TotalMass = TargetPrimitive->Mass + OtherPrimitive->Mass;
        float M1Ratio = OtherPrimitive->Mass / TotalMass;
        float M2Ratio = TargetPrimitive->Mass / TotalMass;

        // 겹친 만큼 서로 반대 방향으로 밀기
        TargetPrimitive->Location += NormalVector * Overlap * M1Ratio;
        OtherPrimitive->Location -= NormalVector * Overlap * M2Ratio;

 

        // Impulse 구하기
        FVector vRel = TargetPrimitive->Velocity - OtherPrimitive->Velocity;  // 상대 속도
        float VelAlongNormal = vRel.Dot(NormalVector);    // 법선 방향(충돌 축)의 속도 성분

        // 두 공이 충돌 상태가 아님(멀어지고 있음)
        if (VelAlongNormal > 0.0f)
        {
            return;
        }

        // 충돌 확정 -> 이펙트 발생
        FVector CollisionPoint = (TargetPrimitive->Location + OtherPrimitive->Location) * 0.5f;
        UEffectManager::GetInstance().PlayEffect(
            "Resources/collision.png",
            DirectX::XMFLOAT2(CollisionPoint.x, CollisionPoint.y),
            0.25f,
            2.0f,
            7,
            false,
            0.0f
        );

        // 선형 속도
        // 충격량 계산
        // j = -(1 + e) * (vRel· n) / (1 / m1 + 1 / m2)
        float jNormal = -(1.0f + TargetPrimitive->Elastic) * VelAlongNormal;
        jNormal /= (1.0f / TargetPrimitive->Mass + 1.0f / OtherPrimitive->Mass);

        // 충격량 벡터 생성
        FVector NormalImpulse = NormalVector * jNormal;

        // 운동량 변화 적용 ( a = F / m)

        TargetPrimitive->Velocity += NormalImpulse / TargetPrimitive->Mass;
        OtherPrimitive->Velocity -= NormalImpulse / OtherPrimitive->Mass; // 작용 반장욕

        // 각속도 처리
        if (TargetPrimitive->bEnableAngularVelocity)
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
                FVector R1 = NormalVector * TargetPrimitive->Radius;
                FVector R2 = NormalVector * -OtherPrimitive->Radius;

                // 각속도 적용
                TargetPrimitive->AngularVelocity += R1.Cross(FrictionImpulse) / TargetPrimitive->Inertia;
                OtherPrimitive->AngularVelocity += R2.Cross(FrictionImpulse * -1.0f) / OtherPrimitive->Inertia; // 작용 반작용으로 반대 방향
            }
        }
    }
    
}
