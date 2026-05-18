# Unreal C++ 구현 체크리스트

<aside>
<img src="https://www.notion.so/icons/code_blue.svg" alt="https://www.notion.so/icons/code_blue.svg" width="40px" />

이 문서는 “포트폴리오 관점에서 설득력 있는” **언리얼 C++ 구현 체크리스트**입니다.

완성 후 스크린샷/코드 캡처/설명에 그대로 활용할 수 있게 구성했습니다.

</aside>

## 1. 모듈/책임 분리

- Character
    - 이동/입력/애니메이션 상태 전환(최소)
- Component
    - StatComponent(HP/Stamina/Groggy)
    - CombatComponent(무기 판정/콤보/큐)
- Interface
    - CombatInterface(피격/상호작용)
- AI
    - BTTask(공격 실행)
    - Decorator(페이즈 체크)
    - Service(거리/라인오브사이트 업데이트)

---

## 2. 구현 항목 체크리스트

### 1) 입력(Enhanced Input)

- [ ]  이동/카메라
- [ ]  약공/강공
- [ ]  회피
- [ ]  패링
- [ ]  상호작용(E) (그로기 처형)

### 2) 전투 판정

- [ ]  무기 충돌(AnimNotifyState로 활성/비활성)
- [ ]  Trace 기반(선택): Capsule/Sphere Sweep로 안정성 확보
- [ ]  Friendly fire 방지(소유자 무시)
- [ ]  히트 중복 방지(한 공격 당 1회만)

### 3) 스탯/상태 머신

- [ ]  HP 감소/사망 처리
- [ ]  Stamina 소모/회복 딜레이
- [ ]  Groggy 누적/발동/리셋
- [ ]  플레이어 상태(Enum): Idle/Attack/Dodge/Parry/Hit/Dead
- [ ]  보스 상태(Enum): Idle/Attack/Groggy/PhaseChange/Dead

### 4) UI (Event-driven)

- [ ]  HP/스태미나/그로기 변경 Delegate
- [ ]  UMG 바인딩 최소화(필요할 때만 SetText/SetPercent)
- [ ]  보스 HP 표시

### 5) AI (Behavior Tree)

- [ ]  타깃 추적(MoveTo + 회전 보간)
- [ ]  패턴 선택(거리/쿨다운)
- [ ]  페이즈 전환 데코레이터(옵저빙)
- [ ]  패턴 실행 태스크(몽타주/노티파이 연동)

### 6) 최적화/안정성

- [ ]  `TObjectPtr` + `UPROPERTY()`로 참조 안정성 확보
- [ ]  Tick 최소화: 필요 시 Timer/Delegate 기반으로 전환
- [ ]  데이터 드리븐: DT/DA로 수치 관리

---

## 3. 데모 완성 기준(개발 관점)

- “플레이어 입력 → 판정 → 스탯 변화 → UI 반영”이 완전히 연결됨
- “보스 패턴 → 대응(회피/패링) → 그로기 → 처형” 루프가 1사이클 이상 동작
- 코드/폴더 구조가 설명 가능한 수준으로 정리됨 (README로 요약 가능)

---

## 4. 산출물(포트폴리오용)

- 짧은 영상: P1~P2 전환 + 처형 1회 포함
- 코드 캡처: Interface/Delegate/BTTask/Decorator 핵심 부분
- 1페이지 요약: 시스템 구조(다이어그램) + 주요 기술 포인트