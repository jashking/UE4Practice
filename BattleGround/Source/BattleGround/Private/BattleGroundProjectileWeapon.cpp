// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleGroundProjectileWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

void ABattleGroundProjectileWeapon::Fire()
{
	AActor* Owner = GetOwner();
	if (Owner && ProjectileClass)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		const FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
	}
}
