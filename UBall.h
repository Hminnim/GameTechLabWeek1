#pragma once

#include "UPrimitive.h"
#include "CollisionManager.h"
#include "UGameManager.h"

enum class USkillType
{
    ReverseMagnet,
    Mine,
    Freeze,
    SizeScaling,
    MassScaling
};

class UBall : public UPrimitive
{
public:
    static int TotalNumBalls;

    std::string                 m_textureKey;
	ID3D11ShaderResourceView*   m_textureView;

	ID3D11Buffer*   m_vertexBuffer;
	UINT            m_numVertices;

    bool bEnableFreeze = false;
    bool isFreezed = false;
    bool isSelfDestruct = false;
    bool isSizeScaling = false;
    bool isMassScaling = false;
    bool isMagnetActivated = false;
    bool AlreadyActiveMag = false;

    EPlayer Owner = EPlayer::Red;

    float TargetRadius;
    float TargetMass;

    UBall(const std::string& meshKey, const EPlayer owner, const FVector startLocation);
    virtual ~UBall();
    virtual void Render(URenderer& Renderer) override;
    virtual void ApplyGravity(float DeltaTime) override;
    virtual void SetGNumber(float NewG);
    virtual void SetElastic(float NewElastic) override;
    virtual void SetTexture(ID3D11ShaderResourceView* srv);
    virtual void ApplyReverseMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce) override;
    virtual void ApplySelfFreeze() override;
    virtual void ApplyAirResistance(float DeltaTime, float AirResistance) override;
    virtual void SetEnableAngularMomentum(bool bEnable) override;
    virtual void ApplySkill(USkillType Skill);
    virtual void Update(float DeltaTime, std::vector<UPrimitive*>& others) override;

};