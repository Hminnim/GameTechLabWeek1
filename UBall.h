#pragma once

#include "UPrimitive.h"
#include "CollisionManager.h"
#include "UGameManager.h"

class UBall : public UPrimitive
{
public:
    static int TotalNumBalls;

    std::string                 m_textureKey;
	ID3D11ShaderResourceView*   m_textureView;

	ID3D11Buffer*   m_vertexBuffer;
	UINT            m_numVertices;

    bool bEnableShotgun = false;
    bool bEnableFreeze = false;
    bool bEnableWallCreate = false;
    bool isFreezed = false;
    bool isShotgunbullet = false;
    FVector ShotgunstartPos;
    bool isSelfDestruct = false;
    bool isSizeScaling = false;
    bool isMassScaling = false;
    bool isMagnetActivated = false;   
    bool AlreadyActiveMag = false;
    bool isGiantActivated = false;      // 지름증가 스킬 활성화 여부
    bool isHeavierActivated = false;    // 질량증가 스킬 활성화 여부


    // Freeze Effect용 직전 프레임 freeze 감지 변수
    bool bWasFreezed = false;
    // Skill별 Aura Effect 구분 key
    char _skillAuraKey = 0; 


    EPlayer Owner = EPlayer::Red;

    float TargetRadius;
    float TargetMass;

    int currentWallCount = 0;
    int MaxWallCount = 20;

    UBall(const std::string& meshKey, const EPlayer owner, const FVector startLocation);
    virtual ~UBall();
    virtual void Render(URenderer& Renderer) override;
    virtual void SetElastic(float NewElastic) override;
    virtual void SetTexture(ID3D11ShaderResourceView* srv);
    virtual void ApplyReverseMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce, float MaxDist) override;
    virtual void ApplySelfFreeze() override;
    virtual void SetEnableAngularMomentum(bool bEnable) override;
    virtual void ApplySkill(ESkillType Skill);
    virtual void RemoveAllSkill();
    virtual void Update(float DeltaTime, std::vector<UPrimitive*>& others) override;
    void CollisionManage(float DeltaTime, std::vector<UPrimitive*>& others);
    void ReverseMagnetWhenMine(float DeltaTime, std::vector<UPrimitive*>& others);
    void FrictionFloor(float DeltaTime, std::vector<UPrimitive*>& others);
    void SizeMassScaling(float DeltaTime);
    void WallCollision();
    void WallCreate();
    
};