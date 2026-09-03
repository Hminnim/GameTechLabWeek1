# 🎯 커스텀 2D 물리 엔진 게임 (알까기)

C++17, Win32 API 및 DirectX 11을 사용하여 밑바닥부터(From Scratch) 직접 개발한 2D 턴제 물리 전략 게임입니다. 자체 개발한 물리 엔진을 통해 정밀한 원-원(Circle-to-Circle) 및 원-벽(Circle-to-Wall) 충돌에 따른 충격량(Impulse) 연산을 처리하며, 전략적인 "스킬 드래프트(Skill Draft)" 시스템을 결합하여 게임의 깊이를 더했습니다.

## 🛠 사용 기술 (Tech Stack)

* **Language:** C++17
* **Graphics API:** DirectX 11 (DirectX Tool Kit의 SpriteBatch 활용)
* **Platform:** Windows Desktop (Win32 API)
* **Audio:** FMOD Engine
* **UI/Tooling:** 자체 UI 프레임워크 (`UUI`) 및 ImGui

## ✨ 주요 기능 (Core Features)

### 1\. 자체 개발 2D 물리 엔진 (`CollisionManager`)

* **정밀한 충돌 처리:** 독자적인 `FVector` 수학 구조체를 활용하여 충격량(Impulse) 기반의 물리적 충돌 반응을 구현했습니다
* **물리적 속성 적용:** 각 오브젝트의 질량(Mass), 반지름(Radius), 탄성(Elastic) 값을 바탕으로 궤적과 속도를 계산합니다.
* **마찰력 및 맵 경계 처리:** 동적인 마찰력 계산을 통해 공이 자연스럽게 멈추도록 구현했으며, 화면 밖으로 나가는 오브젝트에 대한 경계 판정(Boundary clamping)을 안전하게 처리합니다.

### 2\. 전략적 스킬 드래프트 시스템 (`UDraftScene`)

* 두 명의 플레이어(Red와 Blue)가 메인 게임 시작 전 번갈아 가며 스킬을 선택하는 밴픽(Draft) 단계를 진행합니다.
* 선택된 스킬들은 `UGameManager`에 안전하게 저장되며, 인게임 씬(`UInGameScene`) 로드 시 각 플레이어의 UI 슬롯에 동적으로 할당됩니다.
* 동적 중앙 정렬 알고리즘이 적용된 UI와 `UFadeOverlay`를 통한 부드러운 화면 페이드 인/아웃 전환 효과를 지원합니다.

### 3\. 다양한 스킬 시스템 (`UBall::ApplySkill`)

드래프트한 스킬에 따라 게임의 양상이 완전히 변화합니다:

* **Mine (지뢰):** 다른 공과 충돌 시 자폭하며 폭발 범위 내의 오브젝트들을 척력으로 강하게 밀어냅니다.
* **Freeze (빙결):** 대상 공을 제자리에 얼려, 마찰력을 극대화해 움직이지 않는 거대한 장애물로 만듭니다.
* **Giant (거대화) / Heavier (질량 증가):** 시간에 따라 공의 반지름이나 질량을 서서히 증가시켜 물리 충돌에서 압도적인 우위를 점합니다.
* **Repulse (밀어내기) / Magnet (끌어당기기):** 주변 일정 반경 내의 오브젝트들에게 지속적인 척력(`ApplyReverseMagnetism`) 또는 인력(`ApplyMagnetism`)을 가합니다.
* **WallCreate (벽 생성):** 공이 일정 거리 이상 이동할 때마다 궤적을 따라 뒤에 물리적 충돌이 가능한 벽(`UWall`)을 생성합니다.
* **Shotgun (산탄):** 발사 직후 공의 궤적이 흩어지는 여러 개의 작은 발사체로 분열됩니다.
* **Return (귀환):** 발사 위치를 기억해두고 맵 밖으로 떨어졌을 때 파괴되지 않고 원래 위치로 되돌아옵니다.

### 4\. 엔진 아키텍처

* **매니저 패턴 (Manager Pattern):** `UGameManager`, `USceneManager`, `UInputManager`, `UResourceManager`, `USoundManager` 등의 싱글톤 매니저를 통해 전역 상태와 각종 리소스를 독립적으로 분리하여 관리합니다.
* **입력 처리:** Win32의 원시 `WM\\\_MOUSEMOVE` 데이터를 읽어올 때 `GET\\\_X\\\_LPARAM` 매크로를 사용하여 화면 밖 마우스 좌표 오버플로우 문제를 방지하고, 게임 로직에는 일관된 좌표계로 변환하여 전달합니다.
* **이펙트 시스템:** `SpriteBatch`를 기반으로 설계된 `UEffectManager`를 통해 일회성 파티클 애니메이션, 마우스 드래그 조준 화살표, 루프형 스킬 오라(Aura) 이펙트를 효율적으로 제어하고 렌더링합니다.
* **오디오 엔진:** FMOD 오디오 라이브러리를 통합하여 폴더 내의 `.wav`, `.mp3` 등의 사운드 리소스를 일괄 로드하고 재생 및 채널 관리를 수행합니다.

