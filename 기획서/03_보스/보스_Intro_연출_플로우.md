# 보스 등장 연출 (Boss Intro) 플로우

<aside>

**목적:** 안개벽 진입 후 보스 첫인상을 만드는 **Intro 연출 플로우와 구현 규격**을 정의한다.
보스 Intro는 포트폴리오 영상의 첫 장면이자, 플레이어가 "싸우고 싶다"는 감각을 받는 결정적 순간이다.

</aside>

---

## 1. Intro 연출 방식 결정

본 데모는 제작 비용 대비 임팩트가 높은 **인게임 연출(In-World Cinematic)** 방식을 채택한다.

| 방식 | 장점 | 단점 | 채택 여부 |
| --- | --- | --- | --- |
| 풀 컷씬 (LevelSequence) | 연출 퀄리티 높음 | 제작 시간 많이 필요 | 선택(Nice) |
| 인게임 연출 (카메라 애니) | 빠른 구현, 자연스러운 전환 | 컷씬보다 임팩트 약함 | **Must** |
| 즉시 전투 시작 | 구현 최소 | 첫인상 없음 | 사용 안 함 |

---

## 2. Intro 연출 전체 흐름

```
[플레이어가 안개벽 트리거 진입]
    ↓
안개벽 통과 처리
    - 전투 구역 이탈 제한 활성화 (BoxTrigger)
    - 플레이어 입력 잠금 (DisableInput)
    ↓
카메라 Intro 시퀀스 시작 (약 3~5초)
    - 보스 전신 숏 (발 → 상체 → 얼굴 팬업)
    또는
    - 보스가 등을 보이다가 플레이어를 향해 고개 돌림
    ↓
보스 이름 UI 등장 (페이드 인)
    - "보스 이름" 텍스트 + HP 바 슬라이드 인
    ↓
Intro 연출 종료
    - 카메라 원위치 복귀 (보간)
    - 플레이어 입력 잠금 해제 (EnableInput)
    - 보스 AI 활성화 (BehaviorTree RunBehavior)
    ↓
[전투 시작]
```

---

## 3. 보스 Intro 중 보스 상태

| 단계 | 보스 상태 | 행동 |
| --- | --- | --- |
| 안개벽 진입 전 | `Idle` (AI 비활성) | 보스방 중앙에 서 있음. 대기 포즈 또는 Idle 애니 루프. |
| Intro 연출 중 | `Intro` (특수 상태) | 전용 Intro 몽타주 재생 (포효, 무기 들기, 플레이어 응시 등). AI 차단. |
| Intro 종료 후 | `Idle` → BT 활성화 | BT가 `HasTarget = true` 세팅 후 즉시 추적 시작. |

> **설계 의도:** Intro 중 보스가 공격하지 않도록 상태로 AI를 명시적으로 차단.
> `ECharacterState::Intro` 상태에서 `TakeDamage` 무시 처리도 함께 적용 (연출 중 피격 방지).

---

## 4. 카메라 Intro 시퀀스 구현 옵션

### 옵션 A: CameraAnim (빠른 구현, Must)

- `UCameraAnimInst`를 이용한 카메라 애니메이션
- 언리얼 내 간단한 카메라 트랙 작성 가능
- 몽타주 종료 시 자동 복귀

### 옵션 B: LevelSequence (고품질, Nice)

- `ALevelSequenceActor` + `ULevelSequencePlayer`
- 카메라/보스 애니/UI 타임라인을 한 곳에서 관리
- 종료 이벤트(`OnFinished`) 바인딩으로 전투 시작 트리거

```cpp
// LevelSequence 재생 예시
void ABossRoom::PlayBossIntro()
{
    if (IntroSequence)
    {
        FMovieSceneSequencePlaybackSettings Settings;
        SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
            GetWorld(), IntroSequence, Settings, IntroActor);
        SequencePlayer->OnFinished.AddDynamic(this, &ABossRoom::OnIntroFinished);
        SequencePlayer->Play();
    }
}

void ABossRoom::OnIntroFinished()
{
    // 입력 해제 + AI 활성화
    PlayerRef->EnableInput(PlayerController);
    BossRef->GetAIController()->RunBehaviorTree(BossBT);
}
```

---

## 5. 보스 이름 UI 등장 규격

| 항목 | 내용 |
| --- | --- |
| 표시 위치 | 화면 하단 중앙 (소울라이크 관례) |
| 구성 | 보스 이름(대문자) + 얇은 구분선 + HP 바 |
| 등장 애니 | 알파 페이드 인 (0 → 1, 0.5s) |
| 트리거 시점 | Intro 시퀀스 중반부 (카메라가 보스 얼굴 비출 때) |
| 소멸 | 보스 사망 시 페이드 아웃 |

---

## 6. 사운드 연출 가이드

| 시점 | 사운드 |
| --- | --- |
| 안개벽 진입 | 안개/마법 통과음 |
| Intro 시작 | BGM 페이드 아웃 → 보스 테마 페이드 인 |
| 보스 포효/등장 동작 | 보스 보이스 SFX (포효, 발소리 등) |
| 전투 시작 | 보스 BGM 풀 볼륨 |

---

## 7. 튜닝 체크리스트

- [ ]  Intro 중 플레이어 입력이 완전히 차단되는가
- [ ]  Intro 중 보스가 공격하지 않는가 (AI 차단 확인)
- [ ]  Intro 종료 → 전투 시작 전환이 매끄럽게 연결되는가
- [ ]  보스 이름 UI가 Intro 중 자연스러운 타이밍에 등장하는가
- [ ]  보스 재도전 시 Intro를 **스킵**할 수 있는가 (2회차부터 선택 스킵 권장)

> **재도전 스킵 권장:** 같은 Intro를 반복해서 보면 피로감을 줌.
> 보스 사망 후 재도전 시에는 Intro 없이 즉시 전투 시작하거나, 짧은 버전(1~2초)으로 교체하는 것을 권장.
