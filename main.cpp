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

    // Resource Manager
    UResourceManager::GetInstance().Initialize(renderer.Device, VertexBufferSphere, NumVerticesSphere);
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
    UEffectManager::GetInstance().PlayEffect(
        "Resources/shooting.png",
        { 500.0f, 500.0f },
        1.0f,
        2.0f,
        6                
    );

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

        if (UInputManager::GetInstance().IsKeyDown(VK_LBUTTON) && !ImGui::GetIO().WantCaptureMouse)
        {
            POINT mouse = UInputManager::GetInstance().GetMousePos();
            if (USceneManager::GetInstance().GetCurrentScene())
            {
                auto& primitives = USceneManager::GetInstance().GetCurrentScene()->GetPrimitives();
                for (auto* prim : primitives)
                {
                    UBall* ball = dynamic_cast<UBall*>(prim);
                    if (ball)
                    {
                        float dx = ball->Location.x - (float)mouse.x;
                        float dy = ball->Location.y - (float)mouse.y;
                        if (dx * dx + dy * dy < ball->Radius * ball->Radius)
                        {
                            SelectedBall = ball;
                            bIsDragging = true; // ⭐ 드래그 조준 시작!
                            SelectedBall->Velocity = FVector(0, 0, 0); // 조준할 땐 공을 정지
                            break;
                        }
                    }
                }
            }
        }
        UEffectManager::GetInstance().Render(); // EFFECT TEST

        // ImGui
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.
        ImGui::Begin("Jungle Property Window");
        ImGui::Text("Hello Jungle World!");
        if (SelectedBall != nullptr)
        {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "[ Ball Selected! ]");
            ImGui::Text("Radius: %.1f | Mass: %.1f", SelectedBall->Radius, SelectedBall->Mass);
            ImGui::Separator();
            // 💥 5가지 코어 스킬 발동 버튼!
            if (ImGui::Button("1. 자폭 (Mine)"))       SelectedBall->ApplySkill(USkillType::Mine);
            if (ImGui::Button("2. 빙결 (Freeze)"))     SelectedBall->ApplySkill(USkillType::Freeze);
            if (ImGui::Button("3. 거대화 (SizeUp)"))   SelectedBall->ApplySkill(USkillType::SizeScaling);
            if (ImGui::Button("4. 질량증가 (MassUp)")) SelectedBall->ApplySkill(USkillType::MassScaling);
            if (ImGui::Button("5. 척력파 (Magnet)"))   SelectedBall->ApplySkill(USkillType::ReverseMagnet);
            if (ImGui::Button("선택 해제 (Deselect)")) SelectedBall = nullptr;
        }
        else
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Click a ball to select!");
        }
        if (bIsDragging && UInputManager::GetInstance().IsKeyUp(VK_LBUTTON))
        {
            if (SelectedBall != nullptr)
            {
                POINT mouse = UInputManager::GetInstance().GetMousePos();
                // 💥 당긴 방향의 반대 방향으로 발사 벡터 계산 (새총 원리)
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

                    // Shooting Effect 적용을 위한 발사각 계산
                    float launchAngle = atan2f(SelectedBall->Velocity.y, SelectedBall->Velocity.x);

                    UEffectManager::GetInstance().PlayEffect(
                        "Resources/shooting.png",
                        DirectX::XMFLOAT2(SelectedBall->Location.x, SelectedBall->Location.y),
                        0.25f,
                        1.5f,
                        6,
                        false,
                        launchAngle
                    );
                }
            }
            bIsDragging = false; // 드래그 종료
        }
        // main.cpp ImGui 창 안에서
     
        // 마우스 값 보기
        bool bIsLeftPress = UInputManager::GetInstance().IsKeyPress(VK_LBUTTON);
        ImGui::Checkbox("Mouse Left", &bIsLeftPress);
        int MousePointValue[2] = { 0, };
        MousePointValue[0] = UInputManager::GetInstance().GetMousePos().x;
        MousePointValue[1] = UInputManager::GetInstance().GetMousePos().y;
        ImGui::InputInt2("Mouse Point", MousePointValue);

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
