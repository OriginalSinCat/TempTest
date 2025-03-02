//$ Copyright 2015-24, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/FlowProcessor.h"

#include "Frameworks/Flow/Domains/FlowDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecGraphScript.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "Frameworks/Flow/ExecGraph/Tasks/CommonFlowGraphDomain.h"

#include "UObject/Package.h"

DEFINE_LOG_CATEGORY_STATIC(LogGridFlowProcessor, Log, All);

FFlowProcessor::FFlowProcessor() {
    RegisteredDomains.Add(MakeShareable(new FCommonFlowGraphDomain));
}

FFlowProcessorResult FFlowProcessor::Process(UFlowExecScript* ExecScript, const FRandomStream& InRandom, const FFlowProcessorSettings& InSettings) {
    NodeStates.Reset();
    VisitedNodes.Reset();
    NodeExecStages.Reset();

    return ExecuteNode(ExecScript->ResultNode, InRandom, InSettings);
}

bool FFlowProcessor::GetNodeState(const FGuid& NodeId, FFlowExecutionOutput& OutNodeState) {
    FFlowExecutionOutput* SearchResult = NodeStates.Find(NodeId);
    if (!SearchResult) {
        return false;
    }

    OutNodeState = *SearchResult;
    return true;
}

FFlowExecutionOutput* FFlowProcessor::GetNodeStatePtr(const FGuid& NodeId) {
    return NodeStates.Find(NodeId);
}

EFlowTaskExecutionStage FFlowProcessor::GetNodeExecStage(const FGuid& NodeId) {
    const EFlowTaskExecutionStage* SearchResult = NodeExecStages.Find(NodeId);
    return SearchResult ? *SearchResult : EFlowTaskExecutionStage::NotExecuted;
}

void FFlowProcessor::RegisterDomain(const TSharedPtr<IFlowDomain>& InDomain) {
    RegisteredDomains.Add(InDomain);
}

namespace FFlowProcessorLib {
    bool ValidateInput(EFlowExecTaskInputConstraint InputConstraint, int NumInputs, FString& OutErrorMessage, EFlowTaskExecutionResult& OutExecutionResult) {
        if (InputConstraint != EFlowExecTaskInputConstraint::NoInput && NumInputs == 0) {
            OutErrorMessage = "Missing Input";
            OutExecutionResult = EFlowTaskExecutionResult::FailHalt;
            return false;
        }

        if (InputConstraint == EFlowExecTaskInputConstraint::NoInput && NumInputs > 0) {
            OutErrorMessage = "Incoming nodes not allowed";
            OutExecutionResult = EFlowTaskExecutionResult::FailHalt;
            return false;
        }
    
        if (InputConstraint == EFlowExecTaskInputConstraint::SingleInput && NumInputs != 1) {
            OutErrorMessage = "Only one input allowed";
            OutExecutionResult = EFlowTaskExecutionResult::FailHalt;
            return false;
        }

        return true;
    };
}

FFlowProcessorResult FFlowProcessor::ExecuteNode(UFlowExecScriptGraphNode* Node, const FRandomStream& InRandom, const FFlowProcessorSettings& InSettings) {
    check(!VisitedNodes.Contains(Node->NodeId));
    check(!NodeStates.Contains(Node->NodeId));

    {
        EFlowTaskExecutionStage& ExecStage = NodeExecStages.FindOrAdd(Node->NodeId);
        ExecStage = EFlowTaskExecutionStage::WaitingToExecute;
    }

    VisitedNodes.Add(Node->NodeId);

    // Prepare the execution input / output 
    FFlowExecutionInput NodeInput;
    NodeInput.Random = &InRandom;
    NodeInput.Dungeon = InSettings.Dungeon;
    for (const TSharedPtr<IFlowDomain>& RegisteredDomain : RegisteredDomains) {
        NodeInput.RegisteredDomains.Add(RegisteredDomain);
    }
    
    FFlowExecutionOutput NodeOutput;
    if (FFlowProcessorLib::ValidateInput(Node->GetInputConstraint(), Node->IncomingNodes.Num(), NodeOutput.ErrorMessage, NodeOutput.ExecutionResult)) {
        UFlowExecTask* Task{};
        if (const UFlowExecScriptTaskNode* TaskNode = Cast<UFlowExecScriptTaskNode>(Node)) {
            // Duplicate the task so we can apply attribute overrides to it without affecting the source asset
            Task = NewObject<UFlowExecTask>(GetTransientPackage(), TaskNode->Task->GetClass(), NAME_None, RF_NoFlags, TaskNode->Task);
            SetTaskAttributes(Task, InSettings);
            NodeInput.Domain = FindDomain(Task->GetClass());

            Task->LookAhead(NodeInput);
        }
        
        TArray<int> IncomingNodeBranchIndices = Node->SelectIncomingNodeBranches(InRandom);
        for (const int IncomingNodeBranchIdx : IncomingNodeBranchIndices) {
            if (!Node->IncomingNodes.IsValidIndex(IncomingNodeBranchIdx)) {
                continue;
            }
            
            UFlowExecScriptGraphNode* IncomingNode = Node->IncomingNodes[IncomingNodeBranchIdx];
            if (!VisitedNodes.Contains(IncomingNode->NodeId)) {
                const FFlowProcessorResult IncomingResult = ExecuteNode(IncomingNode, InRandom, InSettings);
                if (IncomingResult.ExecResult != EFlowTaskExecutionResult::Success) {
                    return IncomingResult;
                }
            }
            
            if (const FFlowExecutionOutput* IncomingNodeOutput = NodeStates.Find(IncomingNode->NodeId)) {
                NodeInput.IncomingNodeOutputs.Add(*IncomingNodeOutput);
            }
        }

        // Make sure all the incoming nodes succeeded
        bool bIncomingFailRetry = false;
        bool bIncomingFailHalt = false;
        for (const FFlowExecutionOutput& IncomingNodeOutput : NodeInput.IncomingNodeOutputs) {
            if (IncomingNodeOutput.ExecutionResult == EFlowTaskExecutionResult::FailRetry) {
                bIncomingFailRetry = true;
            }
            else if (IncomingNodeOutput.ExecutionResult == EFlowTaskExecutionResult::FailHalt) {
                bIncomingFailHalt = true;
            }
        }

        constexpr FFlowTaskExecutionSettings ExecSettings{};
        if (bIncomingFailHalt) {
            NodeOutput.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
            NodeOutput.State = nullptr;
        }
        else if (bIncomingFailRetry) {
            NodeOutput.ExecutionResult = EFlowTaskExecutionResult::FailRetry;
            NodeOutput.State = nullptr;
        }
        else {
            if (Task) {
                if (NodeInput.Domain.IsValid()) {
                    Task->Execute(NodeInput, ExecSettings, NodeOutput);
                }
                else {
                    NodeOutput.ErrorMessage = "Unsupported Node";
                    NodeOutput.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
                    NodeOutput.State = nullptr;
                }
            }
            else if (Node->IsA<UFlowExecScriptResultNode>()) {
                const FFlowExecutionOutput& IncomingState = NodeInput.IncomingNodeOutputs[0];
                NodeOutput.ExecutionResult = IncomingState.ExecutionResult;
                if (IncomingState.State.IsValid()) {
                    NodeOutput.State = IncomingState.State->Clone();
                }
            }

            EFlowTaskExecutionStage& ExecStage = NodeExecStages.FindOrAdd(Node->NodeId);
            ExecStage = EFlowTaskExecutionStage::Executed;
        }

    }
    
    NodeStates.Add(Node->NodeId, NodeOutput);
    
    FFlowProcessorResult Result;
    Result.ExecResult = NodeOutput.ExecutionResult;
    Result.FailReason = NodeOutput.FailureReason;
    return Result;
}

TSharedPtr<IFlowDomain> FFlowProcessor::FindDomain(const TSubclassOf<UFlowExecTask>& InTaskClass) const {
    for (TSharedPtr<IFlowDomain> Domain : RegisteredDomains) {
        TArray<UClass*> DomainTasks;
        Domain->GetDomainTasks(DomainTasks);
        if (DomainTasks.Contains(InTaskClass)) {
            return Domain;
        }
    }
    return nullptr;
}

void FFlowProcessor::SetTaskAttributes(UFlowExecTask* Task, const FFlowProcessorSettings& InSettings) {
    if (Task->NodeVariableName.Len() > 0) {
        // Override the attributes
        const TMap<FString, FDAAttribute>* SearchResult = InSettings.AttributeList.AttributesByNode.Find(Task->NodeVariableName);
        if (SearchResult) {
            const TMap<FString, FDAAttribute>& NodeAttributes = *SearchResult;
            for (auto& Entry : NodeAttributes) {
                FString ParamName = Entry.Key;
                const FDAAttribute& ParamAttrib = Entry.Value;
                if (!Task->SetParameter(ParamName, ParamAttrib)) {
                    UE_LOG(LogGridFlowProcessor, Error, TEXT("Failed to set Parameter: %s.%s"), *Task->NodeVariableName,
                           *ParamName);
                }
            }
        }

        // Override the serialized attributes
        const FString NodePrefix = Task->NodeVariableName + ".";
        for (const auto& Entry : InSettings.SerializedAttributeList) {
            const FString& FullPath = Entry.Key;
            const FString& Value = Entry.Value;
            if (FullPath.StartsWith(NodePrefix)) {
                FString VariableName = FullPath.Mid(NodePrefix.Len());
                if (!Task->SetParameterSerialized(VariableName, Value)) {
                    UE_LOG(LogGridFlowProcessor, Error, TEXT("Failed to set Parameter: %s"), *FullPath);
                }
            }
        }
    }
}

