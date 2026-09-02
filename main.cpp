#pragma once

#include "pch.h"

#include "URenderer.h"
#include "CollisionManager.h"
#include "UBall.h"
#include "GameTimer.h"
#include "UResourceManager.h"
#include "USoundManager.h"
#include "UInputManager.h"
#include "USceneManager.h"
#include "UWindow.h"
#include "UScene.h"
#include "UGameSetting.h"
#include "UEffectManager.h"
#include "UGameManager.h"

int UBall::TotalNumBalls = 0;

int ListCapacity = 0;

static UBall* SelectedBall = nullptr;
static bool bIsDragging = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
    // SoundManager 초기화 및 재생할 음원 파일 설정
    USoundManager::GetInstance().Init();
    USoundManager::GetInstance().LoadAllSounds("Resources/Sounds/");
	USoundManager::GetInstance().PlaySound("title2");

    // Window
    WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";

    UWindow Window;
    if (!Window.InitializedWindow(hInstance, WindowClass, Title, UGameSetting::GetInstance().ScreendWidth, UGameSetting::GetInstance().ScreenHeight))
    {
        return -1;
    }

    HWND hWnd = Window.hWnd;

    // Renderer Class를 생성합니다.
    URenderer renderer;
    renderer.Create(hWnd);
    renderer.CreateShader();
    renderer.CreateConstantBuffer();

    renderer.m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(renderer.DeviceContext);

    // Buffer
    ID3D11Buffer* VertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));
    UINT NumVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);

    ID3D11Buffer* VertexBufferSquare = renderer.CreateVertexBuffer(square_vertices, sizeof(square_vertices));
    UINT NumVerticesSquare = sizeof(square_vertices) / sizeof(FVertexSimple);

    // Resource Manager
    UResourceManager::GetInstance().Initialize("sphere",renderer.Device, VertexBufferSphere, NumVerticesSphere);
    UResourceManager::GetInstance().Initialize("square",renderer.Device, VertexBufferSquare, NumVerticesSquare);
    ID3D11ShaderResourceView* testUITexture = UResourceManager::GetInstance().GetTexture("Resources/Title.png");

    // ImGui를 생성합니다.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init((void*)hWnd);
    ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

    // DeltaTime
    GameTimer Timer;
    float DeltaTime = 0.0f;

    bool bActiveMagnetism = true;
	
    // Resource Manager test code >> 삭제예정
    ID3D11ShaderResourceView* testTexture = UResourceManager::GetInstance().GetTexture("Resources/test.jpg");
    if (!testTexture) {
        OutputDebugStringA("Texture Load Failed!\n");
        assert(false);
    }
    renderer.DeviceContext->PSSetShaderResources(0, 1, &testTexture);
    // << 삭제예정

    // Scene Manager
    USceneManager::GetInstance().AddScene("Title", new UTitleScene());
	USceneManager::GetInstance().AddScene("InGame", new UInGameScene());
	USceneManager::GetInstance().AddScene("GameOver", new UGameOverScene());
    USceneManager::GetInstance().ChangeScene("Title");

    ///////////////////////////////////////////////
    //////////////////EFFECT TEST//////////////////
    ///////////////////////////////////////////////

    UEffectManager::GetInstance().Init(renderer.DeviceContext);
    // UEffectManager::GetInstance().PlayEffect(
    //     "Resources/shooting.png",
    //     { 500.0f, 500.0f },
    //     1.0f,
    //     2.0f,
    //     6                
    // );

    ///////////////////////////////////////////////
    //////////////////EFFECT TEST//////////////////
    ///////////////////////////////////////////////

    bool bIsExit = false;

	// Main Loop (Quit Message가 들어오기 전까지 아래 Loop를 무한히 실행하게 됨)
    while (bIsExit == false)
    {
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
		// SoundManager 업데이트
        USoundManager::GetInstance().Update();
        DeltaTime = Timer.GetDeltaTime();
        UInputManager::GetInstance().Update();

        ////////////////////////////////////////////
        // 매번 실행되는 코드를 여기에 추가합니다.
        USceneManager::GetInstance().Update(DeltaTime);
        
        UEffectManager::GetInstance().Update(DeltaTime); // EFFECT TEST

        renderer.Prepare();
        renderer.PrepareShader();
		USceneManager::GetInstance().Render(renderer);

        //if (UGameManager::GetInstance().CurrentTurnState == ETurnState::WaitInput)
        //{
        //    // 1. 공 선택 하기
        //    if (UInputManager::GetInstance().IsKeyDown(VK_LBUTTON) && !ImGui::GetIO().WantCaptureMouse)
        //    {
        //        POINT mouse = UInputManager::GetInstance().GetMousePos();
        //        if (USceneManager::GetInstance().GetCurrentScene())
        //        {
        //            auto& primitives = USceneManager::GetInstance().GetCurrentScene()->GetPrimitives();
        //            for (auto* prim : primitives)
        //            {
        //                UBall* ball = dynamic_cast<UBall*>(prim);
        //                if (ball && UGameManager::GetInstance().CanSelectBall(ball))
        //                {
        //                    float dx = ball->Location.x - (float)mouse.x;
        //                    float dy = ball->Location.y - (float)mouse.y;
        //                    if (dx * dx + dy * dy < ball->Radius * ball->Radius)
        //                    {
        //                        SelectedBall = ball; // 선택만!
        //                        break;
        //                    }
        //                }
        //            }
        //        }
        //    }
        //    // 2. 우클릭 드래그: 발사 조작 (스킬 걸고 나서 쏘기!)
        //    if (SelectedBall != nullptr && UInputManager::GetInstance().IsKeyDown(VK_RBUTTON) && !ImGui::GetIO().WantCaptureMouse)
        //    {
        //        bIsDragging = true;
        //        SelectedBall->Velocity = FVector(0, 0, 0);
        //    }

        //    // 3. 드래그 중이면 매 프레임 화살표 update 하다가, 마우스 클릭 떼면 발사
        //    if (bIsDragging && SelectedBall != nullptr)
        //    {
        //        POINT mouse = UInputManager::GetInstance().GetMousePos();
        //        FVector launchVec(SelectedBall->Location.x - (float)mouse.x, SelectedBall->Location.y - (float)mouse.y, 0.0f);
        //        float pullDistance = launchVec.Length();
        //        
        //        if (UInputManager::GetInstance().IsKeyUp(VK_RBUTTON))
        //        {
        //            // 발사 처리
        //            if (pullDistance > 5.0f)
        //            {
        //                float launchPower = 8.0f;
        //                SelectedBall->Velocity = launchVec * launchPower;
        //                float maxSpeed = 3000.0f;
        //                if (SelectedBall->Velocity.Length() > maxSpeed)
        //                {
        //                    SelectedBall->Velocity = (SelectedBall->Velocity / SelectedBall->Velocity.Length()) * maxSpeed;
        //                }

        //                // Shooting Effect 적용을 위한 발사각 계산 (좌클릭 -> 우클릭으로 변경)
        //                float launchAngle = atan2f(SelectedBall->Velocity.y, SelectedBall->Velocity.x);

        //                UEffectManager::GetInstance().PlayEffect(
        //                    "Resources/shooting.png",
        //                    DirectX::XMFLOAT2(SelectedBall->Location.x, SelectedBall->Location.y),
        //                    0.25f,
        //                    DirectX::XMFLOAT2(2.0f, 2.0f),
        //                    6,
        //                    false,
        //                    launchAngle
        //                );
        //            }

        //            SelectedBall = nullptr;
        //            UGameManager::GetInstance().CurrentTurnState = ETurnState::BallMoving;
        //            bIsDragging = false;
        //            UEffectManager::GetInstance().ClearArrow(); // 발사 완료 및 화살 삭제
        //        }
        //        else if (pullDistance > 5.0f)
        //        {
        //            // 드래그 중
        //            float launchAngle = atan2f(launchVec.y, launchVec.x);
        //            std::string arrowTexture = (UGameManager::GetInstance().CurrentPlayerTurn == EPlayer::Red)
        //                                        ? "Resources/Red_arrow.png"
        //                                        : "Resources/Blue_arrow.png";


        //            // 조준 방향
        //            FVector aimDir = launchVec / pullDistance;
        //            DirectX::XMFLOAT2 arrowPos =
        //            {
        //                SelectedBall->Location.x + aimDir.x * SelectedBall->Radius,
        //                SelectedBall->Location.y + aimDir.y * SelectedBall->Radius
        //            };
        //            
        //            // 당긴 거리에 비례하여 화살표 크기 조절
        //            float arrowScale = pullDistance / 200.0f;
        //            float minScale = 0.5f;
        //            float maxScale = 2.5f;
        //            arrowScale = std::clamp(arrowScale, minScale, maxScale);
        //            
        //            UEffectManager::GetInstance().DrawArrow(
        //                arrowTexture,
        //                arrowPos,
        //                launchAngle,
        //                1.0f,   // loop duration 
        //                DirectX::XMFLOAT2(arrowScale, 1.0f),
        //                30      // frame count
        //            );
        //        }
        //    }
        //    else
        //    {
        //        UEffectManager::GetInstance().ClearArrow();
        //    }
        //}
        
        UEffectManager::GetInstance().Render(); // EFFECT TEST

        // ImGui
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.
        ImGui::Begin("Jungle Property Window");
        ImGui::Text("Hello Jungle World!");
        //if (SelectedBall != nullptr)
        //{
        //    ImGui::TextColored(ImVec4(0, 1, 0, 1), "[ Ball Selected! ]");
        //    ImGui::Text("Radius: %.1f | Mass: %.1f", SelectedBall->Radius, SelectedBall->Mass);
        //    ImGui::Separator();
        //    // 6가지 코어 스킬 발동 버튼!
        //    if (ImGui::Button("1. 자폭 (Mine)"))       SelectedBall->ApplySkill(USkillType::Mine);
        //    if (ImGui::Button("2. 빙결 (Freeze)"))     SelectedBall->ApplySkill(USkillType::Freeze);
        //    if (ImGui::Button("3. 거대화 (SizeUp)"))   SelectedBall->ApplySkill(USkillType::SizeScaling);
        //    if (ImGui::Button("4. 질량증가 (MassUp)")) SelectedBall->ApplySkill(USkillType::MassScaling);
        //    if (ImGui::Button("5. 척력파 (Magnet)"))   SelectedBall->ApplySkill(USkillType::ReverseMagnet);
        //    if (ImGui::Button("6. 벽 생성 (Wall)"))   SelectedBall->ApplySkill(USkillType::WallCreate);
        //    if (ImGui::Button("선택 해제 (Deselect)")) SelectedBall = nullptr;
        //}
        //else
        //{
        //    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Click a ball to select!");
        //}
        //if (bIsDragging && UInputManager::GetInstance().IsKeyUp(VK_LBUTTON))
        //{
        //    if (SelectedBall != nullptr)
        //    {
        //        POINT mouse = UInputManager::GetInstance().GetMousePos();
        //        FVector launchVec(SelectedBall->Location.x - (float)mouse.x, SelectedBall->Location.y - (float)mouse.y, 0.0f);

        //        float pullDistance = launchVec.Length();
        //        if (pullDistance > 5.0f)
        //        {
        //            float launchPower = 8.0f; // 파워 배율 (조절 가능)
        //            SelectedBall->Velocity = launchVec * launchPower;
        //            // 너무 세게 날아가지 않도록 최대 속도 제한 (최대 3000 px/s)
        //            float maxSpeed = 3000.0f;
        //            if (SelectedBall->Velocity.Length() > maxSpeed)
        //            {
        //                SelectedBall->Velocity = (SelectedBall->Velocity / SelectedBall->Velocity.Length()) * maxSpeed;
        //            }

        //            // Shooting Effect 적용을 위한 발사각 계산
        //            // float launchAngle = atan2f(SelectedBall->Velocity.y, SelectedBall->Velocity.x);

        //            // UEffectManager::GetInstance().PlayEffect(
        //            //     "Resources/shooting.png",
        //            //     DirectX::XMFLOAT2(SelectedBall->Location.x, SelectedBall->Location.y),
        //            //     0.25f,
        //            //     1.5f,
        //            //     6,
        //            //     false,
        //            //     launchAngle
        //            // );
        //        }

        //        UGameManager::GetInstance().CurrentTurnState = ETurnState::BallMoving;
        //    }
        //    bIsDragging = false; // 드래그 종료
        //}
        // main.cpp ImGui 창 안에서
     
        // 마우스 값 보기
        bool bIsLeftPress = UInputManager::GetInstance().IsKeyPress(VK_LBUTTON);
        ImGui::Checkbox("Mouse Left", &bIsLeftPress);
        int MousePointValue[2] = { 0, };
        MousePointValue[0] = UInputManager::GetInstance().GetMousePos().x;
        MousePointValue[1] = UInputManager::GetInstance().GetMousePos().y;
        ImGui::InputInt2("Mouse Point", MousePointValue);

        ImGui::Separator();
        ImGui::Text("=== Game Manager Status ===");

        if (USceneManager::GetInstance().GetCurrentSceneName() == "InGame")
        {
            // 처음 Red Turn Effect 적용을 위한 boolean
            static bool bFirstTurn = true;

            // Turn Effect 적용을 위한 previous turn 변수
            static EPlayer previousTurn = EPlayer::Red;

            // 1. 현재 턴 가져오기
            EPlayer currentTurn = UGameManager::GetInstance().CurrentPlayerTurn;

            // 첫 차례거나, 차례가 바뀌면 true
            bool bTurnChanged = bFirstTurn || (previousTurn != currentTurn);

            if (currentTurn == EPlayer::Red)
            {
                // Red 턴이면 빨간색 텍스트
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[ Turn : RED Player ]");
                
                // Turn이 Blue -> Red 순간에만 Effect 발생
                if (bTurnChanged)
                {
                    UEffectManager::GetInstance().PlayEffect(
                        "Resources/Red_turn.png", // source path
                        { UGameSetting::GetInstance().ScreendWidth * 0.5f, UGameSetting::GetInstance().ScreenHeight * 0.5f }, // size
                        1.0f, // duration
                        DirectX::XMFLOAT2(1.0f, 1.0f), // scale
                        1     // frame count
                    );
                }
            }
            else
            {
                // Blue 턴이면 파란색 텍스트
                ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "[ Turn : BLUE Player ]");

                // Turn이 Red -> Blue 순간에만 Effect 발생
                if (bTurnChanged)
                {
                    UEffectManager::GetInstance().PlayEffect(
                        "Resources/Blue_turn.png", // source path
                        { UGameSetting::GetInstance().ScreendWidth * 0.5f, UGameSetting::GetInstance().ScreenHeight * 0.5f }, // size
                        1.0f, // duration
                        DirectX::XMFLOAT2(1.0f, 1.0f), // scale
                        1     // frame count    
                    );
                }
            }

            previousTurn = currentTurn;
            bFirstTurn = false;
        }   

        // 2. 현재 상태 가져오기
        ETurnState currentState = UGameManager::GetInstance().CurrentTurnState;
        const char* stateStr = "Unknown";
        switch (currentState)
        {
        case ETurnState::WaitInput:  stateStr = "Waiting for Input..."; break;
        case ETurnState::BallMoving: stateStr = "Balls are Moving!";    break;
        case ETurnState::GameOver:   stateStr = "Game Over!";           break;
        }

        // 노란색 텍스트로 현재 상태 출력
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "State: %s", stateStr);
        ImGui::Separator();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

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
	USoundManager::GetInstance().Release();

	return 0;
}
