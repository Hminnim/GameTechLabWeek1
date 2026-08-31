#include <windows.h>

// D3D 사용에 필요한 라이브러리들을 링크합니다.
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D 사용에 필요한 헤더파일들을 포함합니다.
#include <d3d11.h>
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


// 각종 메시지를 처리할 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }

    switch (message)
    {
    case WM_DESTROY:
        // Signal that the app should quit
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

struct FVertexSimple
{
    float x, y, z;    // Position
    float r, g, b, a; // Color
};

// Structure for a 3D vector
struct FVector
{
    float x, y, z;
    FVector(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

    // 연산자 오버로딩
    FVector operator+(const FVector& v) const { return FVector(x + v.x, y + v.y, z + v.z); }
    FVector operator-(const FVector& v) const { return FVector(x - v.x, y - v.y, z - v.z); }
    FVector operator*(float s) const { return FVector(x * s, y * s, z * s); }
    FVector operator/(float s) const { return FVector(x / s, y / s, z / s); }
    void operator+=(const FVector& v) { x += v.x; y += v.y; z += v.z; }
    void operator-=(const FVector& v) { x -= v.x; y -= v.y; z -= v.z; }

    float LengthSquared() const
    {
        return x * x + y * y + z * z;
    }
    float Length() const {
        return (float)sqrt(LengthSquared());
    }
    float Dot(const FVector& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    FVector Cross(const FVector& v) const
    {
        return FVector(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
};

#include "Sphere.h"

class URenderer
{
public:
    // Direct3D 11 장치(Device)와 장치 컨텍스트(Device Context) 및 스왑 체인(Swap Chain)을 관리하기 위한 포인터들
    ID3D11Device* Device = nullptr; // GPU와 통신하기 위한 Direct3D 장치
    ID3D11DeviceContext* DeviceContext = nullptr; // GPU 명령 실행을 담당하는 컨텍스트
    IDXGISwapChain* SwapChain = nullptr; // 프레임 버퍼를 교체하는 데 사용되는 스왑 체인

    // 렌더링에 필요한 리소스 및 상태를 관리하기 위한 변수들
    ID3D11Texture2D* FrameBuffer = nullptr; // 화면 출력용 텍스처
    ID3D11RenderTargetView* FrameBufferRTV = nullptr; // 텍스처를 렌더 타겟으로 사용하는 뷰
    ID3D11RasterizerState* RasterizerState = nullptr; // 래스터라이저 상태(컬링, 채우기 모드 등 정의)
    ID3D11Buffer* ConstantBuffer = nullptr; // 쉐이더에 데이터를 전달하기 위한 상수 버퍼

    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear)할 때 사용할 색상 (RGBA)
    D3D11_VIEWPORT ViewportInfo; // 렌더링 영역을 정의하는 뷰포트 정보

    ID3D11VertexShader* SimpleVertexShader;
    ID3D11PixelShader* SimplePixelShader;
    ID3D11InputLayout* SimpleInputLayout;
    unsigned int Stride;

public:
    // 렌더러 초기화 함수
    void Create(HWND hWindow)
    {
        // Direct3D 장치 및 스왑 체인 생성
        CreateDeviceAndSwapChain(hWindow);

        // 프레임 버퍼 생성
        CreateFrameBuffer();

        // 래스터라이저 상태 생성
        CreateRasterizerState();

        // 깊이 스텐실 버퍼 및 블렌드 상태는 이 코드에서는 다루지 않음
    }

    // Direct3D 장치 및 스왑 체인을 생성하는 함수
    void CreateDeviceAndSwapChain(HWND hWindow)
    {
        // 지원하는 Direct3D 기능 레벨을 정의
        D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

        // 스왑 체인 설정 구조체 초기화
        DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
        swapchaindesc.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
        swapchaindesc.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
        swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
        swapchaindesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
        swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
        swapchaindesc.BufferCount = 2; // 더블 버퍼링
        swapchaindesc.OutputWindow = hWindow; // 렌더링할 창 핸들
        swapchaindesc.Windowed = TRUE; // 창 모드
        swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식

        // Direct3D 장치와 스왑 체인을 생성
        D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
            featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
            &swapchaindesc, &SwapChain, &Device, nullptr, &DeviceContext);

        // 생성된 스왑 체인의 정보 가져오기
        SwapChain->GetDesc(&swapchaindesc);

        // 뷰포트 정보 설정
        ViewportInfo = { 0.0f, 0.0f, (float)swapchaindesc.BufferDesc.Width, (float)swapchaindesc.BufferDesc.Height, 0.0f, 1.0f };
    }

    // Direct3D 장치 및 스왑 체인을 해제하는 함수
    void ReleaseDeviceAndSwapChain()
    {
        if (DeviceContext)
        {
            DeviceContext->Flush(); // 남아있는 GPU 명령 실행
        }

        if (SwapChain)
        {
            SwapChain->Release();
            SwapChain = nullptr;
        }

        if (Device)
        {
            Device->Release();
            Device = nullptr;
        }

        if (DeviceContext)
        {
            DeviceContext->Release();
            DeviceContext = nullptr;
        }
    }

    // 프레임 버퍼를 생성하는 함수
    void CreateFrameBuffer()
    {
        // 스왑 체인으로부터 백 버퍼 텍스처 가져오기
        SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

        // 렌더 타겟 뷰 생성
        D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
        framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // 색상 포맷
        framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

        Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
    }

    // 프레임 버퍼를 해제하는 함수
    void ReleaseFrameBuffer()
    {
        if (FrameBuffer)
        {
            FrameBuffer->Release();
            FrameBuffer = nullptr;
        }

        if (FrameBufferRTV)
        {
            FrameBufferRTV->Release();
            FrameBufferRTV = nullptr;
        }
    }

    // 래스터라이저 상태를 생성하는 함수
    void CreateRasterizerState()
    {
        D3D11_RASTERIZER_DESC rasterizerdesc = {};
        rasterizerdesc.FillMode = D3D11_FILL_SOLID; // 채우기 모드
        rasterizerdesc.CullMode = D3D11_CULL_BACK; // 백 페이스 컬링

        Device->CreateRasterizerState(&rasterizerdesc, &RasterizerState);
    }

    // 래스터라이저 상태를 해제하는 함수
    void ReleaseRasterizerState()
    {
        if (RasterizerState)
        {
            RasterizerState->Release();
            RasterizerState = nullptr;
        }
    }

    // 렌더러에 사용된 모든 리소스를 해제하는 함수
    void Release()
    {
        ReleaseRasterizerState();

        // 렌더 타겟을 초기화
        DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

        ReleaseFrameBuffer();
        ReleaseDeviceAndSwapChain();
    }

    // 스왑 체인의 백 버퍼와 프론트 버퍼를 교체하여 화면에 출력
    void SwapBuffer()
    {
        SwapChain->Present(1, 0); // 1: VSync 활성화
    }

    void CreateShader()
    {
        ID3DBlob* vertexshaderCSO;
        ID3DBlob* pixelshaderCSO;

        D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexshaderCSO, nullptr);

        Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &SimpleVertexShader);

        D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelshaderCSO, nullptr);

        Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &SimplePixelShader);

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        Device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), &SimpleInputLayout);

        Stride = sizeof(FVertexSimple);

        vertexshaderCSO->Release();
        pixelshaderCSO->Release();
    }

    void ReleaseShader()
    {
        if (SimpleInputLayout)
        {
            SimpleInputLayout->Release();
            SimpleInputLayout = nullptr;
        }

        if (SimplePixelShader)
        {
            SimplePixelShader->Release();
            SimplePixelShader = nullptr;
        }

        if (SimpleVertexShader)
        {
            SimpleVertexShader->Release();
            SimpleVertexShader = nullptr;
        }
    }

    void Prepare()
    {
        DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

        DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        DeviceContext->RSSetViewports(1, &ViewportInfo);
        DeviceContext->RSSetState(RasterizerState);

        DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);
        DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    }

    void PrepareShader()
    {
        DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
        DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
        DeviceContext->IASetInputLayout(SimpleInputLayout);

        if (ConstantBuffer)
        {
            DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
        }
    }

    void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
    {
        UINT offset = 0;
        DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);

        DeviceContext->Draw(numVertices, 0);
    }

    ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth)
    {
        // 2. Create a vertex buffer
        D3D11_BUFFER_DESC vertexbufferdesc = {};
        vertexbufferdesc.ByteWidth = byteWidth;
        vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
        vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

        ID3D11Buffer* vertexBuffer;

        Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);

        return vertexBuffer;
    }

    void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
    {
        vertexBuffer->Release();
    }

    struct FConstants
    {
        FVector Offset;
        float Scale;
        FVector Rotation;
        float Pad;
    };

    void CreateConstantBuffer()
    {
        D3D11_BUFFER_DESC constantbufferdesc = {};
        constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0; // ensure constant buffer size is multiple of 16 bytes
        constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
        constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);
    }

    void ReleaseConstantBuffer()
    {
        if (ConstantBuffer)
        {
            ConstantBuffer->Release();
            ConstantBuffer = nullptr;
        }
    }

    void UpdateConstant(FVector Offset, float scale, FVector Rotation)
    {
        if (ConstantBuffer)
        {
            D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

            DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
            FConstants* constants = (FConstants*)constantbufferMSR.pData;
            {
                constants->Offset = Offset;
                constants->Scale = scale;
                constants->Rotation = Rotation;
            }
            DeviceContext->Unmap(ConstantBuffer, 0);
        }
    }
};

class UPrimitive
{
public:
    virtual ~UPrimitive() {}
    virtual void Render(URenderer& Renderer) = 0; 
    virtual void Update(float DeltaTime) = 0;
    virtual void ResolveCollision(UPrimitive* OtherPrimitive) = 0;
    virtual void ApplyGravity(float DeltaTime) = 0;
    virtual void SetGNumber(float NewG) = 0;
    virtual void SetElastic(float NewElastic) = 0;
    virtual void ApplyMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce) = 0;
    virtual void ApplyAirResistance(float DeltaTime, float AirResistance) = 0;
    virtual void SetEnableAngularMomentum(bool bEnable) = 0;
};

class UBall : public UPrimitive
{
public:
    FVector Location;
    FVector Velocity;
    FVector Rotation;
    FVector AngularVelocity;
    float Radius;
    float Mass;
    static int TotalNumBalls;

    float Elastic;
    float GNumber;

    float Inertia;

    // States
    bool bEnableAngularVelocity;

    UBall()
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

    virtual ~UBall()
    {
        TotalNumBalls--;
    }

    virtual void Render(URenderer& Renderer) override
    {
        Renderer.UpdateConstant(Location, Radius, Rotation);
    }
    
    virtual void Update(float DeltaTime) override
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

    virtual void ResolveCollision(UPrimitive* OtherPrimitive) override
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

    virtual void ApplyGravity(float DeltaTime) override
    {
        Velocity.y -= 9.8f * GNumber * DeltaTime;
    }

    virtual void SetGNumber(float NewG)
    {
        GNumber = NewG;
    }

    virtual void SetElastic(float NewElastic) override
    {
        Elastic = NewElastic;
    }

    virtual void ApplyMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce) override
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
            Velocity += (ForceVector / Mass) * DeltaTime;
            Other->Velocity -= (ForceVector / Other->Mass) * DeltaTime; // 작용 반작용
        }
    }
    virtual void ApplyAirResistance(float DeltaTime, float AirResistance) override
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

    virtual void SetEnableAngularMomentum(bool bEnable) override
    {
        bEnableAngularVelocity = bEnable;
    }
};

int UBall::TotalNumBalls = 0;

UPrimitive** PrimitiveList = nullptr;
int ListCapacity = 0;

void IncreaseCapacity()
{
    if (UBall::TotalNumBalls >= ListCapacity)
    {
        // 두배로
        int newCapacity = ListCapacity * 2;
        UPrimitive** newList = new UPrimitive * [newCapacity];

        for (int i = 0; i < UBall::TotalNumBalls; i++)
        {
            newList[i] = PrimitiveList[i];
        }

        delete[] PrimitiveList;
        PrimitiveList = newList;
        ListCapacity = newCapacity;
    }
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";

	// 각종 메시지를 처리할 함수인 WndProc의 함수 포인터를 WindowClass 구조체에 넣는다.
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

	// 윈도우 클래스 등록
	RegisterClassW(&wndclass);

	// 1024 x 1024 크기에 윈도우 생성
	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);

    // Renderer Class를 생성합니다.
    URenderer renderer;
    renderer.Create(hWnd);
    renderer.CreateShader();
    renderer.CreateConstantBuffer();

    // ImGui를 생성합니다.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init((void*)hWnd);
    ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

    // Buffer
    UINT NumVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);
    ID3D11Buffer* VertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));

    // PrimitiveList
    ListCapacity = 10;
    PrimitiveList = new UPrimitive * [ListCapacity];
    int TargetNumBalls = 1;
    PrimitiveList[0] = new UBall();

    // States
    bool bEnableChangeElastic = false;
    bool bEnableGravity = true;
    bool bEnableMagnetism = false;
    bool bEnableAirResistance = false;
    bool bEnableMouseInteractMode = false;
    bool bEnableAngularVelocity = false;

    // Values
    float CurrentElastic = 1.0f;
    float CurrentGNumber = 1.0f;
    float CurrentMagneticForce = 0.1f;
    float CurrentAirResistance = 0.5f;
    UBall* SelectedBall = nullptr;

    // DeltaTime
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);

    LARGE_INTEGER CurrentTime, LastTime;
    QueryPerformanceCounter(&LastTime);
    float DeltaTime = 0.0f;

    bool bIsExit = false;
	// Main Loop (Quit Message가 들어오기 전까지 아래 Loop를 무한히 실행하게 됨)
    while (bIsExit == false)
    {
        QueryPerformanceCounter(&CurrentTime);
        DeltaTime = (float)(CurrentTime.QuadPart - LastTime.QuadPart) / Frequency.QuadPart;
        LastTime = CurrentTime;

        MSG msg;

        // 처리할 메시지가 더 이상 없을때 까지 수행
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // 키 입력 메시지를 번역
            TranslateMessage(&msg);
            // 메시지를 적절한 윈도우 프로시저에 전달, 메시지가 위에서 등록한 WndProc 으로 전달
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                bIsExit = true;
                break;
            }
        }

        ////////////////////////////////////////////
        // 매번 실행되는 코드를 여기에 추가합니다.
        for (int i = 0; i < UBall::TotalNumBalls; i++)
        {
            UBall* CurrentBall = (UBall*)PrimitiveList[i];

            if (bEnableGravity)
            {
                CurrentBall->ApplyGravity(DeltaTime);
            }

            if (bEnableAirResistance)
            {
                CurrentBall->ApplyAirResistance(DeltaTime, CurrentAirResistance);
            }

            CurrentBall->Update(DeltaTime);

            for (int j = i + 1; j < UBall::TotalNumBalls; j++)
            {
                // 충돌 처리
                CurrentBall->ResolveCollision(PrimitiveList[j]);

                // 자력 처리
                if (bEnableMagnetism)
                {
                    CurrentBall->ApplyMagnetism(PrimitiveList[j], DeltaTime, CurrentMagneticForce);
                }
            }
        }


        renderer.Prepare();
        renderer.PrepareShader();

        for (int i = 0; i < UBall::TotalNumBalls; i++)
        {
            UBall* ball = (UBall*)PrimitiveList[i];
            ball->Render(renderer);
            renderer.RenderPrimitive(VertexBufferSphere, NumVerticesSphere);
        }


        // ImGui
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.
        ImGui::Begin("Jungle Property Window");
        ImGui::Text("Hello Jungle World!");

        ImGui::Checkbox("Gravity", &bEnableGravity);
        if (bEnableGravity)
        {
            ImGui::SliderFloat("G-Number", &CurrentGNumber, 0.0f, 5.0f);
        }
        ImGui::Checkbox("Elastic", &bEnableChangeElastic);
        if (bEnableChangeElastic)
        {
            ImGui::SliderFloat("Elasticity", &CurrentElastic, 0.0f, 1.1f);
        }
        ImGui::Checkbox("Magnetism", &bEnableMagnetism);
        if (bEnableMagnetism)
        {
            ImGui::SliderFloat("Magnetic Force", &CurrentMagneticForce, 0.0f, 0.5f);
        }
        ImGui::Checkbox("Air Resistance", &bEnableAirResistance);
        if (bEnableAirResistance)
        {
            ImGui::SliderFloat("Air Resistance Force", &CurrentAirResistance, 0.0f, 1.0f);
        }
        // 마우스 상호작용 모드(공 끌고 가기, 마우스 좌표에 소환)
        ImGui::Checkbox("Mouse Interact Mode", &bEnableMouseInteractMode);
        if (bEnableMouseInteractMode)
        {
            POINT MousePos;
            GetCursorPos(&MousePos);            // 모니터 화면 기준 마우스 좌표
            ScreenToClient(hWnd, &MousePos);    // hWnd(현재창)의 왼쪽 상단 기준으로 좌표 변환

            // 화면 크기 1024x1024
            float Width = 1024.0f;
            float Height = 1024.0f;

            // 마우스 좌표를 스크린 사이즈로 나눠서 0~1 사이의 비율로 만듦
            // 범위를 2로 늘리고 -1을 해서 ndc 범위로 만듬
            float NdcX = (float)MousePos.x / Width * 2.0f - 1.0f;
            float NdcY = -((float)MousePos.y / Height * 2.0f - 1.0f);

            // 마우스 왼쪽 클릭 했을때
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.WantCaptureMouse)
            {
                bool bHitBall = false;

                // 마우스 커서가 공 안에 있는지
                for (int i = 0; i < UBall::TotalNumBalls; i++)
                {
                    UBall* ball = (UBall*)PrimitiveList[i];

                    // 공의 위치와 마우스 좌표 사이의 거리
                    float dx = ball->Location.x - NdcX;
                    float dy = ball->Location.y - NdcY;

                    // 두 거리가 해당 공의 반지름 보다 짧을 때
                    if (dx * dx + dy * dy < ball->Radius * ball->Radius)
                    {
                        SelectedBall = ball;
                        bHitBall = true;

                        // 공은 잡힌듯이 멈추기
                        ball->Velocity = FVector(0, 0, 0);

                        break;
                    }
                }

                // 마우스 커서에 공이 안 잡혔을 때(해당 좌표에 소환)
                if (bHitBall == false)
                {
                    IncreaseCapacity();

                    UBall* NewBall = new UBall();

                    // 마우스 커서 위치로
                    NewBall->Location.x = NdcX;
                    NewBall->Location.y = NdcY;

                    PrimitiveList[UBall::TotalNumBalls - 1] = NewBall;
                    TargetNumBalls++;
                }
            }

            // 마우스 왼쪽 클릭 풀 때
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                // 잡은 공 해제
                SelectedBall = nullptr;
            }

            // 잡은 공이 있고 마우스 왼쪽 클릭을 유지 했을 때
            if (SelectedBall != nullptr && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                // 마우스 위치로 가는 벡터 구하기
                FVector TargetPos(NdcX, NdcY, 0.0f);
                FVector Dir = TargetPos - SelectedBall->Location;

                // 끌려가는 힘
                float SpringForce = 150.0f;

                // 속도 더하기
                SelectedBall->Velocity += Dir * SpringForce * DeltaTime;

                // 가까히 가면 서서히 줄이기 (Damping)
                SelectedBall->Velocity.x *= 0.9f;
                SelectedBall->Velocity.y *= 0.9f;
            }
        }

        ImGui::Checkbox("Angular Velocity", &bEnableAngularVelocity);

        ImGui::InputInt("Number of Balls", &TargetNumBalls);
        // 최소 1개
        if (TargetNumBalls < 1)
        {
            TargetNumBalls = 1;
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // 변경된 Ball의 숫자 적용
        // 증가 시킬 때
        while (UBall::TotalNumBalls < TargetNumBalls)
        {
            IncreaseCapacity();
            UBall* NewBall = new UBall();
            PrimitiveList[UBall::TotalNumBalls - 1] = NewBall;
        }
        // 감소 시킬 때
        while (UBall::TotalNumBalls > TargetNumBalls)
        {
            int RemoveIndex = rand() % UBall::TotalNumBalls;
            UPrimitive* BallToDelete = PrimitiveList[RemoveIndex];

            // 삭제되는 공이 선택된 공이면 선택 해제 시키기
            if (BallToDelete == SelectedBall)
            {
                SelectedBall = nullptr;
            }

            PrimitiveList[RemoveIndex] = PrimitiveList[UBall::TotalNumBalls - 1];
            PrimitiveList[UBall::TotalNumBalls - 1] = nullptr;

            delete BallToDelete;
        }

        // ImGui로 변경한 값들 적용
        for (int i = 0; i < UBall::TotalNumBalls; i++)
        {
            UBall* CurrentBall = (UBall*)PrimitiveList[i];

            if (bEnableChangeElastic)
            {
                CurrentBall->SetElastic(CurrentElastic);
            }
            else
            {
                CurrentBall->SetElastic(1.0f); // Default Elastic
            }

            if (bEnableGravity)
            {
                CurrentBall->SetGNumber(CurrentGNumber);
            }

            CurrentBall->SetEnableAngularMomentum(bEnableAngularVelocity);
        }

        // 현재 화면에 보여지는 버퍼와 그리기 작업을 위한 버퍼를 서로 교환합니다.
        renderer.SwapBuffer();
    }

    // 여기에서 ImGui 소멸
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // 소멸하는 코드를 여기에 추가합니다.
    renderer.ReleaseVertexBuffer(VertexBufferSphere);

    renderer.ReleaseConstantBuffer();
    renderer.ReleaseShader();
    renderer.Release();

    // Primitive 메모리 해제
    int CurrrentBallCount = UBall::TotalNumBalls;
    for (int i = 0; i < CurrrentBallCount; i++)
    {
        if (PrimitiveList[i])
        {
            delete PrimitiveList[i];
        }
    }

    delete[] PrimitiveList;

	return 0;
}
