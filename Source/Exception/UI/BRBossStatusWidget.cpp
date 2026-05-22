#include "BRBossStatusWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UBRBossStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildNativeLayout();
}

void UBRBossStatusWidget::ClearBosses()
{
	RebuildBossRows(0);
	BP_ClearBosses();
}

void UBRBossStatusWidget::SetBossCount(int32 BossCount)
{
	RebuildBossRows(BossCount);
	BP_SetBossCount(BossCount);
}

void UBRBossStatusWidget::SetBossHP(int32 BossIndex, FText BossName, float CurrentHP, float MaxHP, float NormalizedHP)
{
	const float ClampedHP = FMath::Clamp(NormalizedHP, 0.0f, 1.0f);
	if (BossNameTexts.IsValidIndex(BossIndex) && BossNameTexts[BossIndex])
	{
		BossNameTexts[BossIndex]->SetText(BossName);
	}

	if (BossHPBars.IsValidIndex(BossIndex) && BossHPBars[BossIndex])
	{
		BossHPBars[BossIndex]->SetPercent(ClampedHP);
	}

	BP_SetBossHP(BossIndex, BossName, CurrentHP, MaxHP, ClampedHP);
}

void UBRBossStatusWidget::SetBossGroggy(int32 BossIndex, float CurrentGroggy, float MaxGroggy, float NormalizedGroggy)
{
	const float ClampedGroggy = FMath::Clamp(NormalizedGroggy, 0.0f, 1.0f);
	if (BossGroggyBars.IsValidIndex(BossIndex) && BossGroggyBars[BossIndex])
	{
		BossGroggyBars[BossIndex]->SetPercent(ClampedGroggy);
	}

	BP_SetBossGroggy(BossIndex, CurrentGroggy, MaxGroggy, ClampedGroggy);
}

void UBRBossStatusWidget::SetBossGroggyState(int32 BossIndex, bool bIsGroggy)
{
	if (BossGroggyStateTexts.IsValidIndex(BossIndex) && BossGroggyStateTexts[BossIndex])
	{
		BossGroggyStateTexts[BossIndex]->SetText(bIsGroggy ? FText::FromString(TEXT("STUN")) : FText::GetEmpty());
		BossGroggyStateTexts[BossIndex]->SetColorAndOpacity(bIsGroggy ? FSlateColor(FLinearColor(1.0f, 0.75f, 0.2f)) : FSlateColor(FLinearColor::Transparent));
	}

	BP_SetBossGroggyState(BossIndex, bIsGroggy);
}

void UBRBossStatusWidget::BuildNativeLayout()
{
	if (!WidgetTree || BossListBox)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BossStatusRoot"));
	RootBorder->SetPadding(FMargin(0.0f));
	RootBorder->SetBrushColor(FLinearColor::Transparent);
	WidgetTree->RootWidget = RootBorder;

	USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("BossStatusSize"));
	RootSizeBox->SetMinDesiredWidth(760.0f);
	RootBorder->SetContent(RootSizeBox);

	BossListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BossStatusList"));
	RootSizeBox->AddChild(BossListBox);
}

void UBRBossStatusWidget::RebuildBossRows(int32 BossCount)
{
	BuildNativeLayout();
	if (!WidgetTree || !BossListBox)
	{
		return;
	}

	const int32 ClampedBossCount = FMath::Max(0, BossCount);
	if (BossNameTexts.Num() == ClampedBossCount)
	{
		return;
	}

	BossListBox->ClearChildren();
	BossNameTexts.Reset();
	BossGroggyStateTexts.Reset();
	BossHPBars.Reset();
	BossGroggyBars.Reset();

	for (int32 BossIndex = 0; BossIndex < ClampedBossCount; ++BossIndex)
	{
		UVerticalBox* BossRow = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("BossRow_%d"), BossIndex));
		if (UVerticalBoxSlot* RowSlot = BossListBox->AddChildToVerticalBox(BossRow))
		{
			RowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
			RowSlot->SetHorizontalAlignment(HAlign_Fill);
		}

		UHorizontalBox* NameLine = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *FString::Printf(TEXT("BossNameLine_%d"), BossIndex));
		BossRow->AddChildToVerticalBox(NameLine);

		UTextBlock* NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("BossName_%d"), BossIndex));
		NameText->SetText(FText::FromString(FString::Printf(TEXT("Boss %d"), BossIndex + 1)));
		NameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		NameText->SetShadowOffset(FVector2D(1.0f, 1.0f));
		NameText->SetShadowColorAndOpacity(FLinearColor::Black);
		if (UHorizontalBoxSlot* NameSlot = NameLine->AddChildToHorizontalBox(NameText))
		{
			NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		UTextBlock* GroggyStateText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("BossGroggyState_%d"), BossIndex));
		GroggyStateText->SetText(FText::GetEmpty());
		GroggyStateText->SetColorAndOpacity(FSlateColor(FLinearColor::Transparent));
		GroggyStateText->SetShadowOffset(FVector2D(1.0f, 1.0f));
		GroggyStateText->SetShadowColorAndOpacity(FLinearColor::Black);
		if (UHorizontalBoxSlot* StateSlot = NameLine->AddChildToHorizontalBox(GroggyStateText))
		{
			StateSlot->SetHorizontalAlignment(HAlign_Right);
		}

		USizeBox* HPSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("BossHPSize_%d"), BossIndex));
		HPSizeBox->SetWidthOverride(760.0f);
		HPSizeBox->SetHeightOverride(18.0f);
		UProgressBar* HPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), *FString::Printf(TEXT("BossHPBar_%d"), BossIndex));
		HPBar->SetPercent(1.0f);
		HPBar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.05f, 0.04f));
		HPSizeBox->AddChild(HPBar);
		if (UVerticalBoxSlot* HPSlot = BossRow->AddChildToVerticalBox(HPSizeBox))
		{
			HPSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 3.0f));
			HPSlot->SetHorizontalAlignment(HAlign_Fill);
		}

		USizeBox* GroggySizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("BossGroggySize_%d"), BossIndex));
		GroggySizeBox->SetWidthOverride(760.0f);
		GroggySizeBox->SetHeightOverride(8.0f);
		UProgressBar* GroggyBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), *FString::Printf(TEXT("BossGroggyBar_%d"), BossIndex));
		GroggyBar->SetPercent(0.0f);
		GroggyBar->SetFillColorAndOpacity(FLinearColor::White);
		GroggySizeBox->AddChild(GroggyBar);
		if (UVerticalBoxSlot* GroggySlot = BossRow->AddChildToVerticalBox(GroggySizeBox))
		{
			GroggySlot->SetHorizontalAlignment(HAlign_Fill);
		}

		BossNameTexts.Add(NameText);
		BossGroggyStateTexts.Add(GroggyStateText);
		BossHPBars.Add(HPBar);
		BossGroggyBars.Add(GroggyBar);
	}
}
