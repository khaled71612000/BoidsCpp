#include "Boid.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "FlockSubsystem.h"
#include "BoidVolumeSpawner.h"

#include "Components/BoxComponent.h" 

ABoid::ABoid()
{
    PrimaryActorTick.bCanEverTick = false;

    BoidSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Boid Skeletal Mesh"));
    RootComponent = BoidSkeletalMesh;
    BoidSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BoidSkeletalMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

    BoidStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Boid Static Mesh"));
    BoidStaticMesh->SetupAttachment(RootComponent);
    BoidStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BoidStaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

    BoidSkeletalMesh->SetGenerateOverlapEvents(false);
    BoidSkeletalMesh->SetCanEverAffectNavigation(false);

    BoidStaticMesh->SetGenerateOverlapEvents(false);
    BoidStaticMesh->SetCanEverAffectNavigation(false);

    BoidVelocity = FVector::OneVector;
    BoidAcceleration = FVector::ZeroVector;
}

void ABoid::BeginPlay()
{
    Super::BeginPlay();

    FlockSubsystem = GetWorld()->GetSubsystem<UFlockSubsystem>();

    BoidVelocity = FMath::VRand();
    BoidVelocity *= FMath::RandRange(MinAlignForce, MaxAlignForce);
}

void ABoid::SimulateFlock(float DeltaTime)
{
    ComputeFlockForces();
    ApplyFlock(DeltaTime);
}

void ABoid::ComputeFlockForces()
{
    Flock();
}

void ABoid::ApplyFlock(float DeltaTime)
{
    BoidVelocity += BoidAcceleration * DeltaTime;
    BoidVelocity = BoidVelocity.GetClampedToSize(MinAlignForce, MaxAlignForce);

    if (bDisableZ)
    {
        BoidVelocity.Z = 0.0f;
    }

    const FVector newLoc = GetActorLocation() + BoidVelocity * DeltaTime;
    SetActorLocation(newLoc);
    CheckBounds();
    UpdateRotation(DeltaTime);
}

void ABoid::RequestLaunch(const FVector& LaunchVelocity)
{
    PendingLaunchVelocity = LaunchVelocity;
    bPendingLaunch = true;
}

void ABoid::Flock()
{
    if (!FlockSubsystem)
    {
        return;
    }

    if (bPendingLaunch)
    {
        const float launchSpeed = PendingLaunchVelocity.Size();
        if (launchSpeed > 0.f)
        {
            MaxAlignForce = FMath::Max(MaxAlignForce, launchSpeed);
            BoidVelocity = PendingLaunchVelocity;
        }

        ExternalForce = FVector::ZeroVector;
        BoidAcceleration = FVector::ZeroVector;
        bPendingLaunch = false;
        return;
    }

    BoidAcceleration = ExternalForce;
    ExternalForce = FVector::ZeroVector;

    TArray<ABoid*> neighbors;
    FlockSubsystem->GetNeighbors(this, PerceptionRadius, neighbors);

    if (!bDisableSeparation)
    {
        BoidAcceleration += Separation(neighbors);
    }
    if (!bDisableAlign)
    {
        BoidAcceleration += Align(neighbors);
    }
    if (!bDisableCohesion)
    {
        BoidAcceleration += Cohesion(neighbors);
    }
}

FVector ABoid::Separation(const TArray<ABoid*>& boids)
{
    FVector force = FVector::ZeroVector;
    int32 count = 0;

    for (ABoid* other : boids)
    {
        if (!other || other == this)
        {
            continue;
        }

        FVector diff = GetActorLocation() - other->GetActorLocation();
        float dist = diff.Size();

        if (dist < KINDA_SMALL_NUMBER)
        {
            continue;
        }

        force += diff / dist;
        count++;
    }

    if (count > 0)
    {
        force /= count;
        force.Normalize();
        force *= MaxAlignForce;
    }

    return force;
}

FVector ABoid::Align(const TArray<ABoid*>& boids)
{
    FVector force = FVector::ZeroVector;
    int32 count = 0;

    for (ABoid* other : boids)
    {
        if (!other || other == this)
        {
            continue;
        }

        force += other->GetBoidVelocity();
        count++;
    }

    if (count > 0)
    {
        force /= count;
        force.Normalize();
        force *= MaxAlignForce;
    }

    return force;
}

FVector ABoid::Cohesion(const TArray<ABoid*>& boids)
{
    FVector force = FVector::ZeroVector;
    int32 count = 0;

    for (ABoid* other : boids)
    {
        if (!other || other == this)
        {
            continue;
        }

        force += other->GetActorLocation();
        count++;
    }

    if (count > 0)
    {
        force /= count;
        force -= GetActorLocation();
        force.Normalize();
        force *= MaxAlignForce;
    }

    return force;
}

void ABoid::SetPercipRadius(int32 rad)
{
    PerceptionRadius = rad;
}

void ABoid::SetDisableZ(bool bEnable)
{
	bDisableZ = bEnable;
}

void ABoid::SetRules(bool disableAlign, bool disableCohesion, bool disableSeparation)
{
    bDisableAlign = disableAlign;
    bDisableCohesion = disableCohesion;
    bDisableSeparation = disableSeparation;
}

void ABoid::CheckBounds()
{
    if (!FlockSubsystem)
    {
        return;
    }

    FVector loc = GetActorLocation();
    if (FlockSubsystem->CheckBounds(loc, BoidVelocity, bDisableZ))
    {
        SetActorLocation(loc);
    }
}

void ABoid::UpdateRotation(float dt)
{
    if (BoidVelocity.SizeSquared() > KINDA_SMALL_NUMBER)
    {
        FRotator current = GetActorRotation();
        FRotator target = BoidVelocity.Rotation();
        FRotator newRot = FMath::RInterpTo(current, target, dt, RotationInterpSpeed);
        SetActorRotation(newRot);
    }
}

void ABoid::AddForce(const FVector& force)
{
    ExternalForce += force;
}