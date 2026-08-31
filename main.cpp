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
#include "UBall.h"
#include "GameTimer.h"

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
    bool bEnableReverseMagnetism = false;
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
    GameTimer Timer;
    float DeltaTime = 0.0f;

    bool bActiveMagnetism = true;
    bool bIsExit = false;
	// Main Loop (Quit Message가 들어오기 전까지 아래 Loop를 무한히 실행하게 됨)
    while (bIsExit == false)
    {
        
        DeltaTime = Timer.GetDeltaTime();

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

                if(bEnableReverseMagnetism)
                {
                    CurrentBall->ApplyReverseMagnetism(PrimitiveList[j], DeltaTime, CurrentMagneticForce);
                }
            }
        }

        /*if (SelectedBall != nullptr && ImGui::IsMouseDown(ImGuiMouseButton_Left) && bEnableMagnetism && bActiveMagnetism)
        {

            for (int j = 0; j < UBall::TotalNumBalls; j++)
            {
                if (PrimitiveList[j] != SelectedBall)
                {
                    SelectedBall->ApplyReverseMagnetism(PrimitiveList[j], DeltaTime, CurrentMagneticForce);
                }
            }
            bActiveMagnetism = false;
        }*/

      


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
        ImGui::Checkbox("Reverse Magnetism", &bEnableReverseMagnetism);
        if (bEnableReverseMagnetism)
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
