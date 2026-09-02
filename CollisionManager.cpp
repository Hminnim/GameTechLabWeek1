#include "pch.h"
#include "CollisionManager.h"
#include "UBall.h"
#include "UWall.h"
#include "UEffectManager.h"
#include "USoundManager.h"

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
        if (TargetBall->isSelfDestruct && OtherBall->Owner!=TargetBall->Owner)
        {
            TargetPrimitive->bIsDestroyed = true;
            FVector CollisionPoint = (TargetPrimitive->Location + OtherPrimitive->Location) * 0.5f;
            UEffectManager::GetInstance().PlayEffect(
                "Resources/collision.png",
                DirectX::XMFLOAT2(CollisionPoint.x, CollisionPoint.y),
                0.25f,
                DirectX::XMFLOAT2(2.0f, 2.0f),
                7,
                false,
                0.0f
            );
            USoundManager::GetInstance().PlaySound("hit");
            return;
        }
        else if (OtherBall->isSelfDestruct && OtherBall->Owner != TargetBall->Owner)
        {
            OtherPrimitive->bIsDestroyed = true;
            FVector CollisionPoint = (TargetPrimitive->Location + OtherPrimitive->Location) * 0.5f;
            UEffectManager::GetInstance().PlayEffect(
                "Resources/collision.png",
                DirectX::XMFLOAT2(CollisionPoint.x, CollisionPoint.y),
                0.25f,
                DirectX::XMFLOAT2(2.0f, 2.0f),
                7,
                false,
                0.0f
            );
            USoundManager::GetInstance().PlaySound("hit");
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

bool CollisionManager::DetectWallCollision(UPrimitive* TargetPrimitive, UPrimitive* OtherPrimitive)
{
    UBall* Ball = dynamic_cast<UBall*>(TargetPrimitive);
    UWall* Wall = dynamic_cast<UWall*>(OtherPrimitive);

    if (!Ball || !Wall)
    {
        Ball = dynamic_cast<UBall*>(OtherPrimitive);
        Wall = dynamic_cast<UWall*>(TargetPrimitive);
    }if (!Ball || !Wall) return false;

    float halfpoint = Wall->Width * 0.5f;
    float left = Wall->Location.x - halfpoint;
    float right = Wall->Location.x + halfpoint;
    float top = Wall->Location.y - halfpoint;
    float bottom = Wall->Location.y + halfpoint;

    float closestX;
    float closestY;

    if (Ball->Location.x < left) closestX = left;
    else if (Ball->Location.x > right) closestX = right;
    else closestX = Ball->Location.x;

    if (Ball->Location.y < top) closestY = top;
    else if (Ball->Location.y > bottom) closestY = bottom;
    else closestY = Ball->Location.y;
    
    float dx = Ball->Location.x - closestX;
    float dy = Ball->Location.y - closestY;

    float distsq = (dx * dx + dy * dy);

    return distsq < (Ball->Radius * Ball->Radius);
}

void CollisionManager::ResolveWallCollision(UPrimitive* TargetPrimitive, UPrimitive* OtherPrimitive)
{
    UBall* Ball = dynamic_cast<UBall*>(TargetPrimitive);
    UWall* Wall = dynamic_cast<UWall*>(OtherPrimitive);

    if (!Ball || !Wall)
    {
        Ball = dynamic_cast<UBall*>(OtherPrimitive);
        Wall = dynamic_cast<UWall*>(TargetPrimitive);
    }if (!Ball || !Wall) return;


    if (!DetectWallCollision(TargetPrimitive, OtherPrimitive)) return;
  
    float halfpoint = Wall->Width * 0.5f;
    float left = Wall->Location.x - halfpoint;
    float right = Wall->Location.x + halfpoint;
    float top = Wall->Location.y - halfpoint;
    float bottom = Wall->Location.y + halfpoint;

    float closestX;
    float closestY;

    if (Ball->Location.x < left) closestX = left;
    else if (Ball->Location.x > right) closestX = right;
    else closestX = Ball->Location.x;

    if (Ball->Location.y < top) closestY = top;
    else if (Ball->Location.y > bottom) closestY = bottom;
    else closestY = Ball->Location.y;

    float dx = Ball->Location.x - closestX;
    float dy = Ball->Location.y - closestY;

    float dist = sqrtf(dx * dx + dy * dy);

    if (dist == 0.0f) { dx = 1.0f; dist=1.0f; }
    FVector normal(dx / dist, dy / dist, 0.0f);

    float overlap = Ball->Radius - dist;
    Ball->Location += normal * overlap;

    float velAlongNormal = Ball->Velocity.Dot(normal);
    if (velAlongNormal < 0.0f)
    {
        Ball->Velocity -= normal * (2.0f * velAlongNormal) * Ball->Elastic;
    }
}


