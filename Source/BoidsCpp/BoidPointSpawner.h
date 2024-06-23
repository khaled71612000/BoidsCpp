#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boid.h"
#include "BoidPointSpawner.generated.h"

UCLASS()
class BOIDSCPP_API ABoidPointSpawner : public AActor
{
	GENERATED_BODY()
	
public:
	ABoidPointSpawner();

	UPROPERTY(VisibleAnywhere, Category = "Boid|Components")
	class UBillboardComponent* SpawnPointBillboard;

	UPROPERTY(EditInstanceOnly, Category = "Boid|Spawn")
	int32 BoidSpawnNumber;

	UPROPERTY(EditInstanceOnly, Category = "Boid|Spawn")
	TSubclassOf<ABoid> BoidBp;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void SpawnBoids(int32 NumBoids);

	class UFlockSubsystem* FlockSubsystem;
};
