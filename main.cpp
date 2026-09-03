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

#include "UDraftScene.h"

int UBall::TotalNumBalls = 0;

int ListCapacity = 0;

static UBall* SelectedBall = nullptr;
static bool bIsDragging = false;

static bool g_bHasPlayedChargeSound = false;

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

    // Scene Manager
    USceneManager::GetInstance().AddScene("Intro", new UIntroScene());
    USceneManager::GetInstance().AddScene("Title", new UTitleScene());
    USceneManager::GetInstance().AddScene("Draft", new UDraftScene());
	USceneManager::GetInstance().AddScene("InGame", new UInGameScene());
	USceneManager::GetInstance().AddScene("GameOver", new UGameOverScene());
    USceneManager::GetInstance().RequestChangeScene("Intro");

    UEffectManager::GetInstance().Init(renderer.DeviceContext);

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

        UEffectManager::GetInstance().Render(); // EFFECT TEST

        // ImGui
        //ImGui_ImplDX11_NewFrame();
        //ImGui_ImplWin32_NewFrame();
        //ImGui::NewFrame();

        // 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.
        //ImGui::Begin("Jungle Property Window");
        //ImGui::Text("Hello Jungle World!");

        if (SelectedBall != nullptr)
        {
            //ImGui::TextColored(ImVec4(0, 1, 0, 1), "[ Ball Selected! ]");
            //ImGui::Text("Radius: %.1f | Mass: %.1f", SelectedBall->Radius, SelectedBall->Mass);
            //ImGui::Separator();
            //if (ImGui::Button("1. 자폭 (Mine)"))       SelectedBall->ApplySkill(ESkillType::Mine);
            //if (ImGui::Button("2. 빙결 (Freeze)"))     SelectedBall->ApplySkill(ESkillType::Freeze);
            //if (ImGui::Button("3. 거대화 (SizeUp)"))   SelectedBall->ApplySkill(ESkillType::Giant);
            //if (ImGui::Button("4. 질량증가 (MassUp)")) SelectedBall->ApplySkill(ESkillType::Heavier);
            //if (ImGui::Button("5. 척력파 (Magnet)"))   SelectedBall->ApplySkill(ESkillType::Repulse);
            //if (ImGui::Button("6. 벽 생성 (Wall)"))   SelectedBall->ApplySkill(ESkillType::WallCreate);
            //if (ImGui::Button("7. 산탄 (Shotgun)"))   SelectedBall->ApplySkill(ESkillType::Shotgun);
            //if (ImGui::Button("선택 해제 (Deselect)"))
            //{
            //    UEffectManager::GetInstance().ClearAura(SelectedBall); // Aura Effect 해제
            //    SelectedBall = nullptr;
            //}
        }
        else
        {
            //ImGui::TextColored(ImVec4(1, 1, 0, 1), "Click a ball to select!");
        }

        if (bIsDragging && UInputManager::GetInstance().IsKeyUp(VK_LBUTTON))
        {
            if (SelectedBall != nullptr)
            {                
                POINT mouse = UInputManager::GetInstance().GetMousePos();
                FVector launchVec(SelectedBall->Location.x - (float)mouse.x, SelectedBall->Location.y - (float)mouse.y, 0.0f);

                float pullDistance = launchVec.Length();
                if (pullDistance > 5.0f)
                {
                    float launchPower = 8.0f; // 파워 배율 (조절 가능)
                    SelectedBall->Velocity = launchVec * launchPower;
                    // 너무 세게 날아가지 않도록 최대 속도 제한 (최대 3000 px/s)
                    float maxSpeed = 3000.0f;
                    if (SelectedBall->Velocity.Length() > maxSpeed)
                    {
                        SelectedBall->Velocity = (SelectedBall->Velocity / SelectedBall->Velocity.Length()) * maxSpeed;
                    }
                }

                UGameManager::GetInstance().CurrentTurnState = ETurnState::BallMoving;
            }
            bIsDragging = false; // 드래그 종료
        }
        // main.cpp ImGui 창 안에서
     
        // 마우스 값 보기
        //bool bIsLeftPress = UInputManager::GetInstance().IsKeyPress(VK_LBUTTON);
        //ImGui::Checkbox("Mouse Left", &bIsLeftPress);
        //int MousePointValue[2] = { 0, };
        //MousePointValue[0] = UInputManager::GetInstance().GetMousePos().x;
        //MousePointValue[1] = UInputManager::GetInstance().GetMousePos().y;
        //ImGui::InputInt2("Mouse Point", MousePointValue);

        //ImGui::Separator();
        //ImGui::Text("=== Game Manager Status ===");

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
                //ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[ Turn : RED Player ]");
                
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
                //ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "[ Turn : BLUE Player ]");

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

        // 현재 상태 가져오기
        ETurnState currentState = UGameManager::GetInstance().CurrentTurnState;
        const char* stateStr = "Unknown";
        switch (currentState)
        {
        case ETurnState::WaitInput:  stateStr = "Waiting for Input..."; break;
        case ETurnState::BallMoving: stateStr = "Balls are Moving!";    break;
        case ETurnState::GameOver:   stateStr = "Game Over!";           break;
        }

        // 노란색 텍스트로 현재 상태 출력
        //ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "State: %s", stateStr);
        //mGui::Separator();

        //ImGui::End();

        //ImGui::Render();
        //ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // 현재 화면에 보여지는 버퍼와 그리기 작업을 위한 버퍼를 서로 교환합니다.
        renderer.SwapBuffer();
    }

    // 여기에서 ImGui 소멸
    //ImGui_ImplDX11_Shutdown();
    //ImGui_ImplWin32_Shutdown();
    //ImGui::DestroyContext();

    // 소멸하는 코드를 여기에 추가합니다.
    renderer.ReleaseVertexBuffer(VertexBufferSphere);

    renderer.ReleaseConstantBuffer();
    renderer.ReleaseShader();
    renderer.Release();
	USoundManager::GetInstance().Release();

	return 0;
}
