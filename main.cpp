#pragma once

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

#include "URenderer.h"

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
