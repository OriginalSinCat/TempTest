//$ Copyright 2015-24, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Math/RandomStream.h"

class UGrammarRuleScriptGraphNode;

class DUNGEONARCHITECTRUNTIME_API FMathUtils {
public:
    static FORCEINLINE FVector ToVector(const FIntVector& value) {
        return FVector(value.X, value.Y, value.Z);
    }
    
    static FORCEINLINE FIntVector FloorToInt(const FVector& InVec) {
        return FIntVector(
            FMath::FloorToInt(InVec.X),
            FMath::FloorToInt(InVec.Y),
            FMath::FloorToInt(InVec.Z));
    }
    
    static FIntVector ToIntVector(const FVector& value, bool bRound) {
        if (bRound) {
            return FIntVector(
                FMath::RoundToInt(value.X),
                FMath::RoundToInt(value.Y),
                FMath::RoundToInt(value.Z));
        }
        return FIntVector(
            FMath::FloorToInt(value.X),
            FMath::FloorToInt(value.Y),
            FMath::FloorToInt(value.Z));
    }

    
    static FIntPoint ToIntPoint(const FVector2D& value, bool bRound = false) {
        if (bRound) {
            return FIntPoint(
                FMath::RoundToInt(value.X),
                FMath::RoundToInt(value.Y));
        }
        return FIntPoint(
            FMath::FloorToInt(value.X),
            FMath::FloorToInt(value.Y));
    }

    static FORCEINLINE FVector2D GetRandomDirection2D(FRandomStream& Random) {
        float Angle = Random.FRand() * PI * 2;
        return FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
    }

    static TArray<int32> GetShuffledIndices(int32 Count, const FRandomStream& Random);
    static void GetShuffledIndices(int32 Count, const FRandomStream& Random, TArray<int32>& OutIndices);

    template<typename T>
    static void Swap(T& A, T& B) {
        T Temp = A;
        A = B;
        B = Temp;
    }
    
    template <typename T>
    static T GetRandomItem(TArray<T>& Array, FRandomStream& Random) {
        return Array[Random.RandRange(0, Array.Num() - 1)];
    }

    template <typename T>
    static void Shuffle(TArray<T>& Array, const FRandomStream& Random) {
        int32 Count = Array.Num();
        for (int i = 0; i < Count; i++) {
            int32 j = Random.RandRange(0, Count - 1);
            T Temp = Array[i];
            Array[i] = Array[j];
            Array[j] = Temp;
        }
    }

    static double FindAngle(const FVector2d& InVec);
    static bool CalcCircumCenter(const FVector2d& A, const FVector2d& B, const FVector2d& C, FVector2d& OutCircumCenter, double& OutRadius);
    static bool RayRayIntersection(const FVector2d& AO, const FVector2d& AD, const FVector2d& BO, const FVector2d& BD, double& OutTA, double& OutTB);;
    static double GetLineToPointDist2D(const FVector2d& LineA, const FVector2d& LineB, const FVector2d& Point);
};

// Blur algorithm from: http://blog.ivank.net/fastest-gaussian-blur.html (MIT License)
class DUNGEONARCHITECTRUNTIME_API BlurUtils {
public:

    static void boxBlurH_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r);
    static void boxBlurT_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r);
    static void boxBlur_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r);
    static TArray<int32> boxesForGauss(float sigma, float n); // standard deviation, number of boxes
    static void gaussBlur_4(float* scl, float* tcl, float* weights, int32 w, int32 h, int32 r);
    FORCEINLINE static float BlurRound(float Value) {
        return Value;
    }
};

class DUNGEONARCHITECTRUNTIME_API FColorUtils {
public:
    static FLinearColor BrightenColor(const FLinearColor& InColor, float SaturationMultiplier,
                                      float BrightnessMultiplier);

    static FLinearColor GetRandomColor(const FRandomStream& Random, float Saturation);
};


template<typename T>
class TDASmoothValue {
public:
    void Initialize(const T& InValue) {
        TargetValue = SmoothValue = InValue;
    }

    void Tick(float DeltaTime) {
        if (RemainingTime > 0) {
            float Alpha = FMath::Clamp(DeltaTime / RemainingTime, 0.0f, 1.0f);
            SmoothValue = FMath::Lerp(SmoothValue, TargetValue, Alpha);
            RemainingTime -= DeltaTime;
            if (RemainingTime <= 0) {
                SmoothValue = TargetValue;
                RemainingTime = 0;
            }
        }
    }

    void SetTarget(const T& InValue, bool bInAnimate) {
        if (TargetValue != InValue) {
            TargetValue = InValue;
            if (bInAnimate) {
                RemainingTime = TimeToTarget;
            }
            else {
                SmoothValue = TargetValue;
                RemainingTime = 0;
            }
        }
    }

    T GetTargetValue() const { return TargetValue; }
    T GetSmoothValue() const { return SmoothValue; }
    
    void SetTimeToTarget(float InTimeToTarget) { TimeToTarget = InTimeToTarget; }

    T operator()() const {
        return SmoothValue;
    }
    bool IsTransitioning() const { return RemainingTime > 0; }
    
private:
    T TargetValue;
    T SmoothValue;
    float TimeToTarget = 0.1f;
    float RemainingTime = 0.0f;
};

class DUNGEONARCHITECTRUNTIME_API FRandomUtils {
public:
    static FVector GetPointOnTriangle(const FRandomStream& InRandom, const FVector& P0, const FVector& P1, const FVector& P2);
    static FGuid CreateGuid(const FRandomStream& InRandom);
};

