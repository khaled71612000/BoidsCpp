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

    void SetSpawnVolume(ABoidVolumeSpawner* spawner) { SpawnVolume = spawner; }
    void SetMinMaxForce(float minForce, float maxForce) { MinAlignForce = minForce; MaxAlignForce = maxForce; }
    void SetPercipRadius(int32 rad);
    void SetDisableZ(bool bEnable);
    void SetRules(bool disableAlign, bool disableCohesion, bool disableSeparation);

    float GetMinAlign() const { return MinAlignForce; }
    float GetMaxAlign() const { return MaxAlignForce; }
    float GetRotationInterpSpeed() const { return RotationInterpSpeed; }
    FVector GetBoidVelocity() const { return BoidVelocity; }
    float GetPerceptionRadius() const { return PerceptionRadius; }

    void SimulateFlock(float DeltaTime);
    void ComputeFlockForces();
    void ApplyFlock(float DeltaTime);
    void RequestLaunch(const FVector& LaunchVelocity);
    void AddForce(const FVector& force);

    bool IsDisableZ() const { return bDisableZ; }
    bool IsAlignDisabled() const { return bDisableAlign; }
    bool IsSeparationDisabled() const { return bDisableSeparation; }
    bool IsCohesionDisabled() const { return bDisableCohesion; }

protected:
    virtual void BeginPlay() override;

private:

    void Flock();
    void CheckBounds();
    void UpdateRotation(float deltaTime);

    FVector Separation(const TArray<ABoid*>& boids);
    FVector Align(const TArray<ABoid*>& boids);
    FVector Cohesion(const TArray<ABoid*>& boids);

private:

    UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
    USkeletalMeshComponent* BoidSkeletalMesh = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
    UStaticMeshComponent* BoidStaticMesh = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
    ABoidVolumeSpawner* SpawnVolume = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
    FVector BoidVelocity;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
    FVector BoidAcceleration;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
    FVector ExternalForce = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
    float MinAlignForce = -1.f;

    UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
    float MaxAlignForce = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	FVector PendingLaunchVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	bool bPendingLaunch = false;

    UPROPERTY(EditAnywhere, Category = "Boid|Motion")
    float PerceptionRadius = 100.f;

    UPROPERTY(EditAnywhere, Category = "Boid|Motion")
    float RotationInterpSpeed = 5.f;

    bool bDisableZ = false;
    bool bDisableAlign = false;
    bool bDisableSeparation = false;
    bool bDisableCohesion = false;

    UFlockSubsystem* FlockSubsystem = nullptr;
};
