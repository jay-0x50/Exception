# Boss_Raid

Unreal Engine 5.7 기반의 1:1 보스 레이드 액션 게임 프로토타입입니다.

목표는 3~4개월 안에 "패턴 학습 -> 회피/패링 -> 그로기 -> 처형 -> 페이즈 전환" 흐름이 동작하는 소울라이크 스타일 전투 데모를 완성하는 것입니다.

## Project Goal

- Unreal C++ 중심의 전투 시스템 구현
- 기본 에셋을 활용한 빠른 프로토타입 제작
- 블루프린트는 연출, UI, 애니메이션 연결에 집중
- 포트폴리오에서 코드 구조와 시스템 설계를 설명할 수 있는 결과물 제작

## Current Progress

### Player

- 기본 3D 캐릭터 이동/카메라
- HP / Stamina 기본 구조
- 스태미나 소모 및 자동 회복
- 약공격 / 강공격 / 회피 / 패링 입력 처리
- 회피 무적 시간
- 패링 활성 시간
- 공격 Sphere Sweep 판정
- `TakeDamage()` 기반 피격/사망 처리
- UI/BP 연동을 위한 Delegate 및 Blueprint Event 준비

### Git Setup

- Unreal Engine 생성 폴더 제외
- Git LFS로 `.uasset`, `.umap` 등 바이너리 에셋 관리
- 기획서 Markdown 문서 포함

## Controls

현재 C++에서 기본 테스트용 입력을 런타임으로 등록합니다.

| Action | Key |
| --- | --- |
| Light Attack | Left Mouse Button |
| Heavy Attack | Right Mouse Button |
| Dodge | Left Shift |
| Parry | F |
| Interact | E |

## Tech Stack

- Unreal Engine 5.7
- C++
- Enhanced Input
- Git LFS

## Repository Structure

```text
Boss_Raid/
├── Config/              # Unreal project settings
├── Content/             # Unreal assets, tracked through Git LFS
├── Source/              # C++ source code
├── 기획서/               # Game design documents
├── Boss_Raid.uproject
├── .gitignore
└── .gitattributes
```

Generated folders such as `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`, `.vs`, and `.vscode` are intentionally excluded from Git.

## Development Notes

This project is being developed as a learning and portfolio project.

The main focus is not high-end art quality, but building a clear combat loop and a maintainable Unreal C++ architecture:

- Player combat state
- Stat and stamina logic
- Hit detection
- Boss AI pattern selection
- Groggy and execution system
- Phase transition
- Event-driven UI updates

## Design Documents

Detailed planning documents are included in the `기획서/` folder.

They cover:

- Overall game concept
- 3~4 month milestone plan
- Boss pattern sheet
- Combat system details
- Unreal C++ implementation checklist
- Boss AI API design

## Next Milestone

- Separate player stats into a reusable `StatComponent`
- Add a combat interface for damage and hit interaction
- Create a simple boss actor
- Connect player attacks to boss HP/Groggy
- Add basic boss Phase 1 patterns

