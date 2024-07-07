#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidRing.generated.h"

class USphereComponent;

UCLASS()
class BOIDSCPP_API ABoidRing : public AActor
{
	GENERATED_BODY()

public:
	ABoidRing();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DetectionSphere;

	void InfluenceBoids();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "boids")
	float ProbabilityToGoThrough = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "boids")
	float InfluenceDuration;
};
