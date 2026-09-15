#include "UI/SparkCheckpointNoticeWidget.h"

#include "Components/TextBlock.h"
#include "Checkpoint/SparkCheckpoint.h"

void USparkCheckpointNoticeWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 체크포인트 활성화 전역 델리게이트에 핸들러 함수 바인딩
    ASparkCheckpoint::OnCheckpointActivatedGlobal.AddDynamic(this, &USparkCheckpointNoticeWidget::HandleCheckpointActivated);

    // 활성화 전에는 Collapsed로 숨겨두고, 체크포인트 활성화 시에만 Visible로 전환
    SetVisibility(ESlateVisibility::Collapsed);
}

void USparkCheckpointNoticeWidget::NativeDestruct()
{
    // 위젯 파괴 시 댕글링 포인터 및 메모리 누수를 방지하기 위해 델리게이트 안전 해제
    ASparkCheckpoint::OnCheckpointActivatedGlobal.RemoveDynamic(this, &USparkCheckpointNoticeWidget::HandleCheckpointActivated);

    Super::NativeDestruct();
}

void USparkCheckpointNoticeWidget::HandleCheckpointActivated(FName ActivatedCheckpointId)
{
    if (NoticeText)
    {
        NoticeText->SetText(FText::FromString(TEXT("체크포인트 활성화")));
    }

    // Collapsed 상태로는 애니메이션이 재생돼도 렌더링되지 않으므로 재생 전 Visible로 전환
    SetVisibility(ESlateVisibility::HitTestInvisible);
    BP_OnShowNotice();
}
