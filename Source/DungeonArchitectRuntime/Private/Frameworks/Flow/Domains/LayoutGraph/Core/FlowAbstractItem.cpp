//$ Copyright 2015-24, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/LayoutGraph/Core/FlowAbstractItem.h"

#include "Frameworks/Flow/Domains/LayoutGraph/Core/FlowAbstractGraphUtils.h"

namespace {
    FFlowItemCustomInfo ItemCustomInfo_Enemy("E", FLinearColor::White, FLinearColor::Red);
    FFlowItemCustomInfo ItemCustomInfo_Key("K", FLinearColor::Black, FLinearColor::Yellow);
    FFlowItemCustomInfo ItemCustomInfo_Lock("L", FLinearColor::White, FLinearColor::Blue);
    FFlowItemCustomInfo ItemCustomInfo_Entrance("S", FLinearColor::Black, FLinearColor(0, 0.5f, 0));
    FFlowItemCustomInfo ItemCustomInfo_Exit("G", FLinearColor::White, FLinearColor::Black);
    FFlowItemCustomInfo ItemCustomInfo_Treasure("B", FLinearColor::White, FLinearColor(0.15f, 0.15f, 0.5f));
    FFlowItemCustomInfo ItemCustomInfo_Teleporter("T", FLinearColor::Black, FLinearColor(0, 1, 1));

    FFlowItemCustomInfo GetItemCustomInfo(const UFlowGraphItem* Item) {
        switch (Item->ItemType) {
        case EFlowGraphItemType::Enemy: return ItemCustomInfo_Enemy;
        case EFlowGraphItemType::Key: return ItemCustomInfo_Key;
        case EFlowGraphItemType::Lock: return ItemCustomInfo_Lock;
        case EFlowGraphItemType::Entrance: return ItemCustomInfo_Entrance;
        case EFlowGraphItemType::Exit: return ItemCustomInfo_Exit;
        case EFlowGraphItemType::Treasure: return ItemCustomInfo_Treasure;
        case EFlowGraphItemType::Teleporter: return ItemCustomInfo_Teleporter;

        case EFlowGraphItemType::Custom:
        default:
            return Item->CustomInfo;
        }
    }

    FLinearColor InvertColor(const FLinearColor& Color, float InvertStrength) {
        FLinearColor HSV = Color.LinearRGBToHSV();
        HSV.R = FMath::Fmod(HSV.R + 180, 360);
        //HSV.B = 1.0f - HSV.B;

        HSV.B -= 0.5f;
        HSV.B = -HSV.B;
        HSV.B *= InvertStrength;
        HSV.B += 0.5f;

        return HSV.HSVToLinearRGB();
    }
}

FString FFlowItemVisuals::GetText(const UFlowGraphItem* Item) {
    if (!Item) return "";
    return GetItemCustomInfo(Item).PreviewText;
}

FLinearColor FFlowItemVisuals::GetTextColor(const UFlowGraphItem* Item, bool bInvertColor) {
    if (!Item) return FLinearColor::White;
    FLinearColor Color = GetItemCustomInfo(Item).PreviewTextColor;
    return bInvertColor ? InvertColor(Color, 1.0f) : Color;
}

FLinearColor FFlowItemVisuals::GetBackgroundColor(const UFlowGraphItem* Item, bool bInvertColor) {
    if (!Item) return FLinearColor::Black;
    FLinearColor Color = GetItemCustomInfo(Item).PreviewBackgroundColor;
    return bInvertColor ? InvertColor(Color, 0.25f) : Color;
}

/////////////////////////////// UFlowGraphItem ///////////////////////////////

UFlowGraphItem::UFlowGraphItem() {
    ItemId = FGuid::NewGuid();
}

/////////////////////////////// FFlowGraphItemContainer ///////////////////////////////

UFlowGraphItem* FFlowGraphItemContainer::GetItem() const {
    return FFlowAbstractGraphUtils::FindItem(ItemId, HostNode.IsValid() ? HostNode->NodeItems : HostLink->LinkItems);
}

void* FFlowGraphItemContainer::GetParentObject() const {
    return HostNode.IsValid()
            ? static_cast<void*>(HostNode.Get())
            : static_cast<void*>(HostLink.Get());
}

void UDungeonFlowItemMetadataComponent::SetFlowItem(TWeakObjectPtr<const UFlowGraphItem> InFlowItem) {
    FlowItem = InFlowItem;
    OnFlowItemUpdated.Broadcast(InFlowItem.Get());
}

