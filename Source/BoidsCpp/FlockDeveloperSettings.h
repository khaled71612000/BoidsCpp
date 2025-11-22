#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FlockDeveloperSettings.generated.h"

UENUM()
enum class EFlockSimulationMode : uint8
{
    Actors    UMETA(DisplayName="Actor Boids"),
    DataOriented UMETA(DisplayName="Data Oriented Boids"),
};

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Flock"))
class BOIDSCPP_API UFlockDeveloperSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
	static const UFlockDeveloperSettings* Get()
	{
		return GetDefault<UFlockDeveloperSettings>();
	}

    UPROPERTY(EditAnywhere, Config, Category = "Octree", meta = (ClampMin = "1"))
    int32 MaxBoidsPerNode = 8;

    UPROPERTY(EditAnywhere, Config, Category = "Octree", meta = (ClampMin = "1"))
    int32 MaxTreeDepth = 7;

    UPROPERTY(EditAnywhere, Config, Category = "Octree", meta = (ClampMin = "0.01"))
    float MinNodeHalfSize = 1.f;

    UPROPERTY(EditAnywhere, Config, Category = "Flock")
    EFlockSimulationMode SimulationMode = EFlockSimulationMode::Actors;
};