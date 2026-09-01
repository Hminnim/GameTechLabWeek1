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

int UBall::TotalNumBalls = 0;

int ListCapacity = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
    // SoundManager 초기화 및 재생할 음원 파일 설정
    USoundManager SoundManager;
    SoundManager.Init();
	SoundManager.LoadSound("TestSound", "Resources/AlarmSound.wav");

    // Window
    WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";
    int ScreenWidth = 2040;
    int ScreenHeight = 1400;

    UWindow Window;
    if (!Window.InitializedWindow(hInstance, WindowClass, Title, ScreenWidth, ScreenHeight))
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
        SoundManager.Update();
        DeltaTime = Timer.GetDeltaTime();
        UInputManager::GetInstance().Update();

        ////////////////////////////////////////////
        // 매번 실행되는 코드를 여기에 추가합니다.
        USceneManager::GetInstance().Update(DeltaTime);
        renderer.Prepare();
        renderer.PrepareShader();
		    USceneManager::GetInstance().Render(renderer);


        // ImGui
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.
        ImGui::Begin("Jungle Property Window");
        ImGui::Text("Hello Jungle World!");
        if (ImGui::Button("Sound"))
        {
            SoundManager.PlaySound("TestSound");
        }
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
	SoundManager.Release();

	return 0;
}
