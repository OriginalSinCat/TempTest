//$ Copyright 2015-24, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/FlowAssetBase.h"


void UFlowAssetBase::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) {
    UFlowAssetBase* This = CastChecked<UFlowAssetBase>(InThis);

#if WITH_EDITORONLY_DATA
    Collector.AddReferencedObject(This->ExecEdGraph, This);
#endif	// WITH_EDITORONLY_DATA

    Super::AddReferencedObjects(This, Collector);
}

