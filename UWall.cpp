#include "pch.h"
#include "UWall.h"
#include "UResourceManager.h"

int UWall::TotalNumWalls = 0;

UWall::UWall(const std::string& meshKey,const FVector spawnLocation, float width)
{
    TotalNumWalls++;

    // UPrmitive 멤버 초기화
    Radius = 0.0f;
    Mass = 10000000.0f;
    Elastic = 1.0f;
    GNumber = 0.0f;
    Inertia = 0.0f;
    bEnableAngularVelocity = false;

    Width = width;

    // 벽 스폰 위치
    Location = spawnLocation;

    Velocity = FVector(0, 0, 0);
    Rotation = FVector(0, 0, 0);

    m_vertexBuffer = UResourceManager::GetInstance().GetVertexBuffer(meshKey);
    m_numVertices = UResourceManager::GetInstance().GetNumVertices(meshKey);

    m_textureKey = "Resources/button_repulse.png";
    m_textureView = UResourceManager::GetInstance().GetTexture(m_textureKey);
}

UWall::~UWall()
{
    TotalNumWalls--;
}

void UWall::Render(URenderer& Renderer)
{
    Renderer.UpdateConstant(Location, Width, Rotation);
    Renderer.BindTexture(0, m_textureView);
    Renderer.RenderPrimitive(m_vertexBuffer, m_numVertices);
}

void UWall::Update(float DeltaTime, std::vector<UPrimitive*>& others)
{

}

void UWall::SetTexture(ID3D11ShaderResourceView* srv)
{
    m_textureView = srv;
}