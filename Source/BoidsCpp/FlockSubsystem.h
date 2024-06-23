#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FlockSubsystem.generated.h"

UCLASS()
class BOIDSCPP_API UFlockSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UFlockSubsystem();
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void AddToBoidList(class ABoid* boid);
	void RemoveFromBoidList(class ABoid* boid);
    TSet<class ABoid*> GetBoidList() const { return Boids; };

private:
    TSet<class ABoid*> Boids;
};
