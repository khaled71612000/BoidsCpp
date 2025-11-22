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
	// if true , boids steer through the object and if false they will steer away from it 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "boids")
	bool bGoThroughRing = false;

	void InfluenceBoids();
};
