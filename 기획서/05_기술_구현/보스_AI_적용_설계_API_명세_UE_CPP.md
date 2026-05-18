# 보스 AI 적용 설계 & API 명세 (UE C++)

<aside>
<img src="https://www.notion.so/icons/robot_blue.svg" alt="https://www.notion.so/icons/robot_blue.svg" width="40px" />

이 문서는 “보스 패턴을 **AI가 상황 판단으로 선택/발동**”하기 위한 **언리얼 C++ 설계 + API 명세**입니다.

목표는 구현 난이도를 낮추면서도, 고정 로테이션보다 **예측 난이도/리플레이성**을 높이는 것.

</aside>

## 1. AI 적용 전략 (요약)

### 쿨타임의 의미를 “즉시 사용”이 아닌 “사용 가능(Ready)”로 전환

- 스킬은 쿨타임 종료 시 **Ready**가 됨
- Ready는 “발동 가능 후보”가 되는 것일 뿐, **AI가 상황을 보고 실행**

### 패턴 선택은 2단계로 처리

1) **Filter(가능 후보 필터링)**: 거리/시야/상태/Ready 여부 등으로 후보 축소

2) **Score(가중치/점수화)**: 후보 각각의 “명중/압박 성공 확률”을 점수화 → 최종 선택

---

## 2. 데이터 드리븐 구조 (추천)

### 2-1) BossSkillData (DataAsset 또는 DataTable Row)

스킬 하나를 “데이터로 정의”해두면, 무료 에셋 교체/수치 튜닝이 쉬움.

권장 필드(예시)

- `SkillTag` (FGameplayTag 또는 FName)
- `Cooldown` (float)
- `MinRange` / `MaxRange` (float)
- `MinAngleToTarget` (float, 선택)
- `bRequiresLineOfSight` (bool)
- `AttackType` (Normal/Smash/AOE)
- `BaseWeight` (float) : 기본 출현 가중치
- `RepeatPenalty` (float) : 직전/최근 사용 시 가중치 감소 계수
- `AggroScale` (float) : AggroLevel에 따른 가중치 변화량
- `Montage` (UAnimMontage*) 또는 BP로 연결되는 연출 키
- `DamageProfileId` (FName) : 데미지/그로기/히트스톱 테이블 참조

---

## 3. 컴포넌트 설계 (C++ Core)

### 3-1) UBossSkillComponent (보스 스킬 런타임 관리자)

역할

- 스킬 목록 로드(DA/DT)
- 스킬별 쿨타임/Ready 상태 관리
- 후보 필터링 + 스코어 계산
- 최종 선택된 스킬 실행 요청(몽타주/BTTask로 위임)

#### 주요 상태(예시)

- `TMap<FGameplayTag, float> CooldownRemaining`
- `TMap<FGameplayTag, float> LastUsedTime`
- `FGameplayTag LastUsedSkill`

#### 이벤트(선택)

- `OnSkillBecameReady(SkillTag)`
- `OnSkillExecuted(SkillTag)`

---

## 4. API 명세 (개발용)

> 아래는 “지금 당장 구현 가능한 수준”으로 잡은 최소 API(권장).
> 

> 실제 네이밍은 프로젝트 컨벤션에 맞춰 조정.
> 

### 4-1) UBossSkillComponent.h (Interface)

```cpp
USTRUCT(BlueprintType)
struct FBossContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) float DistToTarget = 0.f;
	UPROPERTY(BlueprintReadWrite) float TargetStaminaRatio = 1.f; // 0~1 (추정값 가능)
	UPROPERTY(BlueprintReadWrite) bool bHasLineOfSight = true;
	UPROPERTY(BlueprintReadWrite) bool bTargetIsGreedy = false; // 공격 지속/근접 유지 등으로 추정
	UPROPERTY(BlueprintReadWrite) float AggroLevel = 0.5f; // 0~1
};

USTRUCT(BlueprintType)
struct FSkillDecision
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGameplayTag SkillTag;
	UPROPERTY(BlueprintReadOnly) float Score = 0.f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UBossSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 쿨타임/Ready 업데이트
	UFUNCTION(BlueprintCallable)
	void TickCooldown(float DeltaSeconds);

	// "지금 상황"에서 사용할 수 있는 후보를 구함
	UFUNCTION(BlueprintCallable)
	void GetCandidates(const FBossContext& Ctx, TArray<FGameplayTag>& OutCandidates) const;

	// 후보를 점수화하여 최종 스킬을 결정
	UFUNCTION(BlueprintCallable)
	bool DecideSkill(const FBossContext& Ctx, FSkillDecision& OutDecision);

	// 스킬 실행 요청(실제 애니/히트박스는 BTTask나 캐릭터가 처리)
	UFUNCTION(BlueprintCallable)
	bool RequestExecuteSkill(FGameplayTag SkillTag);

	// 스킬 실행 성공 시 호출(쿨타임 시작, lastUsed 기록)
	UFUNCTION(BlueprintCallable)
	void NotifySkillExecuted(FGameplayTag SkillTag);

	// 쿨타임/Ready 조회
	UFUNCTION(BlueprintCallable)
	bool IsSkillReady(FGameplayTag SkillTag) const;
};
```

### 4-2) DecideSkill() 점수화 규칙 (예시)

- `Score = BaseWeight`
- 거리 적합도(사거리 안이면 +, 아니면 후보 제외)
- LoS 필요 스킬은 `bHasLineOfSight == false`면 제외
- `bTargetIsGreedy == true`면 카운터/스매시 계열 가중치 증가
- `LastUsedSkill`과 같으면 `Score *= RepeatPenalty` (예: 0.2)
- `Score += AggroScale * AggroLevel`
- 최종 선택은 **가중 랜덤** 또는 **최대 점수** (가중 랜덤 권장)

---

## 5. Behavior Tree 적용 예시

### 권장 구조

- Service: `UpdateBossContext`
    - 거리/시야/플레이어 행동 추정치(탐욕, 스태미나 등) 업데이트
- Task: `BTTask_DecideSkill`
    - SkillComponent.DecideSkill() 호출 → Blackboard에 `NextSkillTag` 세팅
- Task: `BTTask_ExecuteSkill`
    - NextSkillTag에 맞는 몽타주/연출 실행
    - 성공 시 NotifySkillExecuted() 호출

### Blackboard 추천 키

- `DistToTarget` (float)
- `AggroLevel` (float)
- `bTargetIsGreedy` (bool)
- `NextSkillTag` (Name/Tag)
- `LastSkillTag` (Name/Tag)

---

## 6. 플레이어 행동 추정(“탐욕” 판정) 아이디어

무료 에셋 기반이라도 “상황 판단 느낌”을 주는 핵심 장치.

- 플레이어가 보스 근거리에서 **연속 공격 유지 시간**이 길면 `bTargetIsGreedy = true`
- 플레이어가 회피를 연속 사용하면 AggroLevel을 낮추거나(패턴 템포 완화) 반대로 잡기/광역을 섞음
- 패링 성공 횟수가 누적되면 패링 가능한 Normal 비중을 낮추고 Smash 비중을 올림(가중치 조정)

---

## 7. 무료 에셋/디자인 리소스 추천 (어디서 구할지)

### 7-1) 언리얼 엔진 공식

- **Fab (구 Unreal Marketplace)**
    - Free 카테고리/월간 무료(기간 한정) 체크
- **Epic Games ‘Free for the Month’**
    - 월마다 고퀄 에셋이 무료로 풀리기도 함

### 7-2) 캐릭터/애니메이션

- **Mixamo**
    - 인디/프로토타입용 애니메이션 소스 다양 (리깅/다운로드 편함)

### 7-3) 사운드/VFX

- **Freesound** (SFX)
- **OpenGameArt** (SFX/텍스처 등)

### 7-4) UI/아이콘

- [**Kenney.nl**](http://Kenney.nl)
    - UI 아이콘/간단한 게임 리소스 무료 제공

### 7-5) “무료 에셋 사용” 시 체크할 것

- 라이선스(상업/비상업), 크레딧 표기 요구 여부
- 재배포 금지 조항(에셋 원본을 따로 공유하면 안 되는 경우)
- 포트폴리오 공개(유튜브/깃헙/압축파일 배포) 시 허용 범위

<aside>
<img src="https://www.notion.so/icons/robot_blue.svg" alt="boss ai" width="40px" />

**추천 운영:** MVP 단계는 “무료 에셋 + 규칙/판정/AI 품질”에 집중하고, 아트는 최소한으로 통일감만 잡기.

</aside>
