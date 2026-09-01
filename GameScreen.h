#pragma once
#include "UIManager.h"

//메인 화면
class MainScreen : public UIElement
{
    public:
        MainScreen();
        void Render() override;
};

struct ItemSlot
{
    int itemId = -1; // 아이템 번호, -1이면 빈 슬롯
    bool isUsed = false; // 사용 여부

    bool IsAvailable() const;
    void Use();

};

// 아이템 화면
class ItemScreen : public UIElement
{
    public:
        static const int SLOT_COUNT = 5;

        ItemScreen();

        void Init() override;
        void Render() override;

        void UseLeftSlot(int idx);
        void UseRightSlot(int idx);

    private:
        ItemSlot _leftSlots[SLOT_COUNT]; // Player 1 Item slot
        ItemSlot _rightSlots[SLOT_COUNT]; // Player 2 Item slot
};

// 중지 화면
class PauseScreen : public UIElement
{
    public:
        PauseScreen();

        void Show() override; // 중지 버튼 & 사운드 처리
        void Hide() override; // 재개 & 사운드 처리
        void Render() override;

};

// Winner 종류 (Player 1/2)
enum class WinnerType
{
    None,
    Player1,
    Player2
};

// 게임 결과 (Winner 발표)
class ResultScreen :  public UIElement
{
    public:
        ResultScreen();

        void SetWinner(WinnerType winner);
        void Show() override; // Winner 팝업 & 사운드 처리
        void Render() override;

    private:
        WinnerType _winner = WinnerType::None;

};

// Game Initialization (Register all the screens we need)
void InitGameUI();