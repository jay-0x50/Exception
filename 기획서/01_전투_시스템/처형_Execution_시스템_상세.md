# 처형(Execution) 시스템 상세

<aside>

**목적:** 그로기 상태에서 발동되는 **처형(Execution) 시스템의 트리거 조건, 판정 규격, 연출 흐름**을 정의한다.
처형은 보스전의 클라이맥스이자 포트폴리오에서 가장 인상적인 순간이므로, 실패/오발동 없이 확실하게 동작해야 한다.

</aside>

---

## 1. 처형 트리거 조건

처형은 아래 **모든 조건이 동시에 성립**해야 발동한다.

| 조건 | 수치/내용 |
| --- | --- |
| 보스 상태 | `ECharacterState::Groggy` |
| 거리 | 플레이어 ↔ 보스 ≤ `ExecutionRange` (기본값: `200`) |
| 플레이어 상태 | `Idle` 또는 `Moving` (Attack/Dodge/Dead 중에는 불가) |
| 그로기 잔여 시간 | `GroggyTimeRemaining > 0.5s` (처형 모션 시작 직전 체크) |
| 입력 | 상호작용 키 `E` 입력 |

---

## 2. 처형 가능 범위 안내 UI

플레이어가 처형 가능 여부를 직관적으로 알 수 있도록 UX 안내를 제공한다.

| 상태 | 표시 |
| --- | --- |
| 보스 그로기 + 범위 밖 | 보스 머리 위 아이콘 없음 (또는 흐린 아이콘) |
| 보스 그로기 + 범위 안 | 보스 머리 위 처형 아이콘 표시 + `[E] 처형` 프롬프트 |
| 처형 중 | UI 전체 숨김 (연출에 집중) |

> **구현 팁:** 매 Tick이 아닌 보스 그로기 진입 시 `Timer`로 0.2s 간격 거리 체크 후 위젯 가시성 토글.

---

## 3. 처형 발동 실패 케이스

처형 키를 눌렀지만 조건 불충족 시 아래처럼 처리한다.

| 실패 케이스 | 처리 |
| --- | --- |
| 거리 초과 | 아무 반응 없음 (입력 무시). 프롬프트 미표시 상태이므로 자연스럽게 처리. |
| 그로기 시간 만료 직전 (0.5s 미만) | 처형 불가 처리. 보스가 Groggy에서 Idle로 회복하는 중이므로 충돌 방지. |
| 플레이어가 공격/회피 중 | 현재 액션 유지. 처형 입력 무시. |
| 보스 그로기 만료 (처형 미입력) | 보스 `ECharacterState::Idle` 복귀. 그로기 게이지 0으로 리셋. 복귀 몽타주 재생. |

---

## 4. 처형 연출 흐름 (시퀀스)

```
[플레이어 E 입력]
    ↓
조건 체크 (거리/상태/그로기)
    ↓ 성공
플레이어 상태 → ECharacterState::Execution
보스 상태 → ECharacterState::Execution (입력 차단, 위치 고정)
    ↓
플레이어 → 보스 방향으로 즉시 스냅 이동 (보간, 거리 150 지점)
    ↓
처형 몽타주 동기화 재생
    - 플레이어: AM_Player_Execution
    - 보스: AM_Boss_ExecutionReceive
    ↓
카메라 시퀀스 (LevelSequence 또는 CameraAnim)
    - 예: 정면 클로즈업 → 마무리 타격 순간 슬로우 → 복귀
    ↓
처형 몽타주 종료 (AnimNotify: OnExecutionEnd)
    ↓
보스 HP 처형 피해 적용 (고정값 또는 현재 HP의 일정 %)
그로기 게이지 0 리셋
보스 상태 → Idle 복귀 (처형이 보스를 즉사시키지 않는 경우)
플레이어 상태 → Idle 복귀
```

---

## 5. 처형 피해량 규격

처형은 즉사가 아닌 **대량 피해 + 그로기 리셋** 방식을 권장한다.
(즉사 방식은 전투 흐름을 단절시키고 밸런스 조정이 어려움)

| 항목 | 수치 (기본값) | 비고 |
| --- | --- | --- |
| 처형 피해 | 보스 MaxHP의 `30%` | 고정 비율 피해 |
| 그로기 리셋 | 0으로 초기화 | 처형 후 즉시 재누적 가능 |
| 히트스톱 | 없음 (슬로우 모션으로 대체) | 처형 중 SlowMotion 0.3배속 적용 |
| 처형 횟수 제한 | 없음 (그로기 누적마다 재발동 가능) | 밸런스 상 MaxGroggy 수치로 조절 |

---

## 6. C++ 구현 포인트

```cpp
// 처형 거리 체크 + 발동
void AExPlayerCharacter::TryExecute()
{
    if (!IsValid(BossRef)) return;
    if (BossRef->GetCharacterState() != ECharacterState::Groggy) return;

    float Dist = FVector::Dist(GetActorLocation(), BossRef->GetActorLocation());
    if (Dist > ExecutionRange) return;

    if (GetCharacterState() == ECharacterState::Attack ||
        GetCharacterState() == ECharacterState::Dodge) return;

    // 처형 시작
    StartExecution();
}

void AExPlayerCharacter::StartExecution()
{
    SetCharacterState(ECharacterState::Execution);
    BossRef->SetCharacterState(ECharacterState::Execution);

    // 보스 방향으로 스냅 이동
    FVector TargetLoc = BossRef->GetActorLocation()
                        - BossRef->GetActorForwardVector() * 150.f;
    SetActorLocation(TargetLoc, false, nullptr, ETeleportType::TeleportPhysics);

    // 몽타주 재생 (BP에서 처리 가능)
    PlayExecutionMontage();
}
```

---

## 7. 튜닝 체크리스트

- [ ]  처형 범위(200) 안에서 `[E]` 프롬프트가 정확히 표시되는가
- [ ]  처형 발동 시 플레이어가 보스 앞으로 자연스럽게 스냅 이동하는가
- [ ]  두 몽타주(플레이어/보스)가 동기화 없이 엇갈리지 않는가
- [ ]  처형 중 카메라 연출이 UI를 가리지 않고 자연스럽게 전환되는가
- [ ]  처형 종료 후 두 캐릭터 모두 Idle로 정상 복귀하는가
- [ ]  그로기 만료 직전 처형 시도 시 오작동(버그)이 없는가
