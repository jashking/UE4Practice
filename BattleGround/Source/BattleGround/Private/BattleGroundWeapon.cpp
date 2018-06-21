// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleGroundWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "HAL/IConsoleManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"

#include "BattleGround.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVarDebugWeaponDrawing(TEXT("BattleGround.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);

// Sets default values
ABattleGroundWeapon::ABattleGroundWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = TEXT("MuzzleSocket");
	TracerTargetName = TEXT("Target");
}

void ABattleGroundWeapon::StartFire()
{
	const float FirestDelay = FMath::Max<float>(TimeBetweenShots - (GetWorld()->TimeSeconds - LastFireTime), 0.f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ABattleGroundWeapon::Fire, TimeBetweenShots, true, FirestDelay);
}

void ABattleGroundWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ABattleGroundWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

void ABattleGroundWeapon::PlayFireEffects(const FVector& TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		const FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		APlayerController* PC = Cast<APlayerController>(Owner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ABattleGroundWeapon::Fire()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		const FVector ShotDirection = EyeRotation.Vector();
		const FVector TraceEnd = EyeLocation + ShotDirection * 1000;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			float ActualDamage = BaseDamage;
			const EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage += 10;
			}

			UGameplayStatics::ApplyPointDamage(Hit.GetActor(), ActualDamage, ShotDirection, Hit, Owner->GetInstigatorController(), this, DamageType);

			UParticleSystem* SelectedEffect = nullptr;
			switch (SurfaceType)
			{
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			if (SelectedEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
		}

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.f, 1.f);
		}

		PlayFireEffects(Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd);

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

