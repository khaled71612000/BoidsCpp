#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boid.generated.h"


class USphereComponent;
class UFlockSubsystem;
class UStaticMeshComponent;
class ABoidVolumeSpawner;

UCLASS()
class BOIDSCPP_API ABoid : public AActor
{
	GENERATED_BODY()

public:
	ABoid();

	FVector GetBoidVelocity() const { return BoidVelocity; }
	void SetSpawnVolume(ABoidVolumeSpawner* spawner) { SpawnVolume = spawner; }
	void SetMinMaxForce(float minForce, float maxForce)
	{ 
		MinAlignForce = minForce;
		MaxAlignForce = maxForce;
	}

	void SetRules(bool disableAlign, bool disableCohesion, bool disableSeparation)
	{
		bDisableAlign = disableAlign;
		bDisableCohesion = disableCohesion;
		bDisableSeparation = disableSeparation;
	}

	void SetPercipRadius(int32 rad);
	void SetZToggle(bool bEnable) { bDisableZ = bEnable; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	void Flock(float DeltaTime);
	void CheckBounds();
	void UpdateRotation(float DeltaTime);

	FVector Separation(TArray<AActor*> boids);
	FVector Align(TArray<AActor*> boids);
	FVector Cohesion(const TArray<AActor*> boids);

	//UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	//FRotator CurrentRotation = FRotator();

	UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
	UStaticMeshComponent* BoidMesh = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
	ABoidVolumeSpawner* SpawnVolume = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	USphereComponent* DetectionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	FVector BoidVelocity;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	FVector BoidAcceleration;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	float MinAlignForce = -1;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	float MaxAlignForce = 1;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	bool bDisableZ = false;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	bool bDisableAlign = false;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	bool bDisableSeparation = false;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	bool bDisableCohesion = false;

	UFlockSubsystem* FlockSubsystem;
};
