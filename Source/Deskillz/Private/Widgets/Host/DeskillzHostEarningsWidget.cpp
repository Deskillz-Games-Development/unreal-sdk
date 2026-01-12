// =============================================================================
// Deskillz SDK for Unreal Engine - Host Earnings Widget Implementation
// Copyright (c) 2024 Deskillz.Games. All rights reserved.
// =============================================================================

#include "UI/Widgets/Host/DeskillzHostEarningsWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"

void UDeskillzHostEarningsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind time range buttons
    if (DayButton)
    {
        DayButton->OnClicked.AddDynamic(this, &UDeskillzHostEarningsWidget::OnDayRangeClicked);
    }
    if (WeekButton)
    {
        WeekButton->OnClicked.AddDynamic(this, &UDeskillzHostEarningsWidget::OnWeekRangeClicked);
    }
    if (MonthButton)
    {
        MonthButton->OnClicked.AddDynamic(this, &UDeskillzHostEarningsWidget::OnMonthRangeClicked);
    }
    if (YearButton)
    {
        YearButton->OnClicked.AddDynamic(this, &UDeskillzHostEarningsWidget::OnYearRangeClicked);
    }
    if (AllTimeButton)
    {
        AllTimeButton->OnClicked.AddDynamic(this, &UDeskillzHostEarningsWidget::OnAllTimeRangeClicked);
    }

    // Bind action buttons
    if (WithdrawButton)
    {
        WithdrawButton->OnClicked.AddDynamic(this, &UDeskillzHostEarningsWidget::OnWithdrawClicked);
    }
    if (ExportButton)
    {
        ExportButton->OnClicked.AddDynamic(this, &UDeskillzHostEarningsWidget::OnExportClicked);
    }

    // Set default time range
    SetTimeRange(EDeskillzEarningsTimeRange::Week);
}

void UDeskillzHostEarningsWidget::SetEarningsData(const FDeskillzHostEarningsData& InEarningsData)
{
    EarningsData = InEarningsData;
    UpdateAllDisplays();
}

void UDeskillzHostEarningsWidget::SetTimeRange(EDeskillzEarningsTimeRange Range)
{
    CurrentTimeRange = Range;
    UpdateTimeRangeButtonStates();
    OnTimeRangeChanged.Broadcast(Range);
    // Chart data will be updated when new data arrives
}

void UDeskillzHostEarningsWidget::RefreshData()
{
    OnRefreshRequested.Broadcast();
}

void UDeskillzHostEarningsWidget::OnDayRangeClicked()
{
    SetTimeRange(EDeskillzEarningsTimeRange::Day);
}

void UDeskillzHostEarningsWidget::OnWeekRangeClicked()
{
    SetTimeRange(EDeskillzEarningsTimeRange::Week);
}

void UDeskillzHostEarningsWidget::OnMonthRangeClicked()
{
    SetTimeRange(EDeskillzEarningsTimeRange::Month);
}

void UDeskillzHostEarningsWidget::OnYearRangeClicked()
{
    SetTimeRange(EDeskillzEarningsTimeRange::Year);
}

void UDeskillzHostEarningsWidget::OnAllTimeRangeClicked()
{
    SetTimeRange(EDeskillzEarningsTimeRange::AllTime);
}

void UDeskillzHostEarningsWidget::OnWithdrawClicked()
{
    OnWithdrawRequested.Broadcast();
}

void UDeskillzHostEarningsWidget::OnExportClicked()
{
    OnExportRequested.Broadcast();
}

void UDeskillzHostEarningsWidget::UpdateAllDisplays()
{
    UpdateTotalEarningsDisplay();
    UpdatePeriodEarningsDisplay();
    UpdateBreakdownDisplay();
    UpdateChart();
}

void UDeskillzHostEarningsWidget::UpdateTotalEarningsDisplay()
{
    if (TotalEarningsText)
    {
        TotalEarningsText->SetText(FText::FromString(FormatCurrency(EarningsData.TotalEarnings)));
    }
    if (AvailableBalanceText)
    {
        AvailableBalanceText->SetText(FText::FromString(FormatCurrency(EarningsData.AvailableBalance)));
    }
    if (PendingPayoutText)
    {
        PendingPayoutText->SetText(FText::FromString(FormatCurrency(EarningsData.PendingPayout)));
    }
}

void UDeskillzHostEarningsWidget::UpdatePeriodEarningsDisplay()
{
    if (PeriodEarningsText)
    {
        PeriodEarningsText->SetText(FText::FromString(FormatCurrency(EarningsData.PeriodEarnings)));
    }

    // Calculate and display change
    if (PeriodChangeText && EarningsData.PreviousPeriodEarnings > 0.0f)
    {
        float PercentChange = ((EarningsData.PeriodEarnings - EarningsData.PreviousPeriodEarnings) / 
                               EarningsData.PreviousPeriodEarnings) * 100.0f;
        
        FString ChangeStr = FString::Printf(TEXT("%s%.1f%%"), PercentChange >= 0 ? TEXT("+") : TEXT(""), PercentChange);
        PeriodChangeText->SetText(FText::FromString(ChangeStr));
        
        // Set color based on positive/negative
        FSlateColor ChangeColor = PercentChange >= 0 ? 
            FSlateColor(FLinearColor(0.2f, 0.8f, 0.4f, 1.0f)) : 
            FSlateColor(FLinearColor(1.0f, 0.3f, 0.3f, 1.0f));
        PeriodChangeText->SetColorAndOpacity(ChangeColor);
    }

    // Update period label
    if (PeriodLabelText)
    {
        PeriodLabelText->SetText(FText::FromString(GetTimeRangeLabel()));
    }
}

void UDeskillzHostEarningsWidget::UpdateBreakdownDisplay()
{
    if (RakeEarningsText)
    {
        RakeEarningsText->SetText(FText::FromString(FormatCurrency(EarningsData.RakeEarnings)));
    }
    if (BonusEarningsText)
    {
        BonusEarningsText->SetText(FText::FromString(FormatCurrency(EarningsData.BonusEarnings)));
    }
    if (ReferralEarningsText)
    {
        ReferralEarningsText->SetText(FText::FromString(FormatCurrency(EarningsData.ReferralEarnings)));
    }

    // Update percentages
    float Total = EarningsData.RakeEarnings + EarningsData.BonusEarnings + EarningsData.ReferralEarnings;
    if (Total > 0.0f)
    {
        if (RakePercentText)
        {
            RakePercentText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), 
                (EarningsData.RakeEarnings / Total) * 100.0f)));
        }
        if (BonusPercentText)
        {
            BonusPercentText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), 
                (EarningsData.BonusEarnings / Total) * 100.0f)));
        }
        if (ReferralPercentText)
        {
            ReferralPercentText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), 
                (EarningsData.ReferralEarnings / Total) * 100.0f)));
        }
    }
}

void UDeskillzHostEarningsWidget::UpdateChart()
{
    if (!ChartCanvas || EarningsData.ChartDataPoints.Num() == 0) return;

    // Clear existing chart elements
    ChartCanvas->ClearChildren();

    // Find min/max for scaling
    float MinValue = TNumericLimits<float>::Max();
    float MaxValue = TNumericLimits<float>::Min();
    
    for (const FDeskillzEarningsDataPoint& Point : EarningsData.ChartDataPoints)
    {
        MinValue = FMath::Min(MinValue, Point.Value);
        MaxValue = FMath::Max(MaxValue, Point.Value);
    }

    // Add padding to range
    float Range = MaxValue - MinValue;
    if (Range < 0.01f) Range = 1.0f;
    MinValue -= Range * 0.1f;
    MaxValue += Range * 0.1f;

    // Draw chart bars or line based on chart type
    if (bUseLineChart)
    {
        DrawLineChart(MinValue, MaxValue);
    }
    else
    {
        DrawBarChart(MinValue, MaxValue);
    }

    // Update axis labels
    UpdateChartAxisLabels(MinValue, MaxValue);
}

void UDeskillzHostEarningsWidget::UpdateTimeRangeButtonStates()
{
    const FLinearColor ActiveColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);
    const FLinearColor InactiveColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);

    if (DayButton) DayButton->SetColorAndOpacity(CurrentTimeRange == EDeskillzEarningsTimeRange::Day ? ActiveColor : InactiveColor);
    if (WeekButton) WeekButton->SetColorAndOpacity(CurrentTimeRange == EDeskillzEarningsTimeRange::Week ? ActiveColor : InactiveColor);
    if (MonthButton) MonthButton->SetColorAndOpacity(CurrentTimeRange == EDeskillzEarningsTimeRange::Month ? ActiveColor : InactiveColor);
    if (YearButton) YearButton->SetColorAndOpacity(CurrentTimeRange == EDeskillzEarningsTimeRange::Year ? ActiveColor : InactiveColor);
    if (AllTimeButton) AllTimeButton->SetColorAndOpacity(CurrentTimeRange == EDeskillzEarningsTimeRange::AllTime ? ActiveColor : InactiveColor);
}

void UDeskillzHostEarningsWidget::DrawLineChart(float MinValue, float MaxValue)
{
    // Line chart drawing implementation
    // This would create line segments connecting data points
}

void UDeskillzHostEarningsWidget::DrawBarChart(float MinValue, float MaxValue)
{
    // Bar chart drawing implementation
    // This would create vertical bars for each data point
}

void UDeskillzHostEarningsWidget::UpdateChartAxisLabels(float MinValue, float MaxValue)
{
    if (YAxisMaxText)
    {
        YAxisMaxText->SetText(FText::FromString(FormatCurrency(MaxValue)));
    }
    if (YAxisMidText)
    {
        YAxisMidText->SetText(FText::FromString(FormatCurrency((MinValue + MaxValue) / 2.0f)));
    }
    if (YAxisMinText)
    {
        YAxisMinText->SetText(FText::FromString(FormatCurrency(MinValue)));
    }
}

FString UDeskillzHostEarningsWidget::GetTimeRangeLabel() const
{
    switch (CurrentTimeRange)
    {
        case EDeskillzEarningsTimeRange::Day: return TEXT("Today");
        case EDeskillzEarningsTimeRange::Week: return TEXT("This Week");
        case EDeskillzEarningsTimeRange::Month: return TEXT("This Month");
        case EDeskillzEarningsTimeRange::Year: return TEXT("This Year");
        case EDeskillzEarningsTimeRange::AllTime: return TEXT("All Time");
        default: return TEXT("Unknown");
    }
}

FString UDeskillzHostEarningsWidget::FormatCurrency(float Amount) const
{
    if (Amount >= 1000000.0f)
    {
        return FString::Printf(TEXT("$%.1fM"), Amount / 1000000.0f);
    }
    else if (Amount >= 1000.0f)
    {
        return FString::Printf(TEXT("$%.1fK"), Amount / 1000.0f);
    }
    return FString::Printf(TEXT("$%.2f"), Amount);
}