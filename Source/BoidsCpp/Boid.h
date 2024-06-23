#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boid.generated.h"

UCLASS()
class BOIDSCPP_API ABoid : public AActor
{
	GENERATED_BODY()

public:
	ABoid();

	FVector GetBoidVelocity() const { return BoidVelocity; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void Flock(float DeltaTime);
	void UpdateRotation(float DeltaTime);
	FVector Align(TSet<ABoid*> boids);

	class UFlockSubsystem* FlockSubsystem;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	FVector BoidVelocity;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
	int32 MinAlignForce = 0;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
	int32 MaxAlignForce = 300;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
	int32 MaxSpeed = 50;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
	float PerceptionRadius = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
	UStaticMeshComponent* BoidMesh = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Boid|Motion")
	FRotator CurrentRotation = FRotator();
};
