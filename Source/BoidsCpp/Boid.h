#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boid.generated.h"

class USphereComponent;
class UFlockSubsystem;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class ABoidVolumeSpawner;

UCLASS()
class BOIDSCPP_API ABoid : public AActor
{
    GENERATED_BODY()

public:
    ABoid();

    FVector GetBoidVelocity() const { return BoidVelocity; }
    float GetPerceptionRadius() const { return PerceptionRadius; }

    void AddForce(const FVector& force);
    void SetSpawnVolume(ABoidVolumeSpawner* spawner) { SpawnVolume = spawner; }
    float GetMaxAlign() const { return MaxAlignForce; }
    void SetMinMaxForce(float minForce, float maxForce) { MinAlignForce = minForce; MaxAlignForce = maxForce; }
    void SetPercipRadius(int32 rad);
    void SetZToggle(bool bEnable) { bDisableZ = bEnable; }
    void SetRules(bool disableAlign, bool disableCohesion, bool disableSeparation);

    void ResetInfluenceState();
    bool bIsInfluencedByRing = false;
    FTimerHandle InfluenceTimerHandle;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:

    void Flock(float deltaTime);
    void CheckBounds();
    void UpdateRotation(float deltaTime);

    FVector Separation(const TArray<ABoid*>& boids);
    FVector Align(const TArray<ABoid*>& boids);
    FVector Cohesion(const TArray<ABoid*>& boids);

    void TriggerInfluence(float duration);

private:

    UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
    USkeletalMeshComponent* BoidSkeletalMesh = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
    UStaticMeshComponent* BoidStaticMesh = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
    ABoidVolumeSpawner* SpawnVolume = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
    FVector BoidVelocity;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
    FVector BoidAcceleration;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
    float MinAlignForce = -1.f;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
    float MaxAlignForce = 1.f;

    UPROPERTY(EditAnywhere, Category = "Boid|Motion")
    float PerceptionRadius = 100.f;

    bool bDisableZ = false;
    bool bDisableAlign = false;
    bool bDisableSeparation = false;
    bool bDisableCohesion = false;

    UFlockSubsystem* FlockSubsystem = nullptr;
};
