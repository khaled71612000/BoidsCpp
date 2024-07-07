#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidVolumeSpawner.generated.h"

class UBillboardComponent;
class UBoxComponent;
class UFlockSubsystem;
class ABoid;

UCLASS()
class BOIDSCPP_API ABoidVolumeSpawner : public AActor
{
    GENERATED_BODY()
    
public:    
    ABoidVolumeSpawner();
protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    UBoxComponent* GetSpawnVolume() const { return SpawnVolume; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid")
    int32 BoidSpawnNumber;

    UPROPERTY(EditAnywhere, Category = "Boid")
    TSubclassOf<ABoid> BoidBp;

private:
    void SpawnBoids(int32 NumBoids);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBillboardComponent* SpawnPointBillboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* SpawnVolume;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
	float PerceptionRadius = 100.f;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
    float MinAlignForce = -1;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
    float MaxAlignForce = 1;

    UPROPERTY(EditAnywhere, Category = "Boid|Motion")
    bool bDisableZ = false;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
	bool bDisableAlign = false;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
	bool bDisableSeparation = false;

	UPROPERTY(EditAnywhere, Category = "Boid|Motion")
	bool bDisableCohesion = false;

    UFlockSubsystem* FlockSubsystem;
};
