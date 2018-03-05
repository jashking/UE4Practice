// Fill out your copyright notice in the Description page of Project Settings.

#include "FovCharacter.h"

#include "EngineUtils.h"

// Sets default values
AFovCharacter::AFovCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFovCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	for (TActorIterator<AFogOfWar> Iter(GetWorld()); Iter; ++Iter)
	{
		FogOfWar = *Iter;
		break;
	}

	if (FogOfWar.IsValid())
	{
		FogOfWar->AddFovObject(this);
	}
}

// Called every frame
void AFovCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	static double LastUpdateTime = FPlatformTime::Seconds();
	if (FPlatformTime::Seconds() - LastUpdateTime > 1.f)
	{
		LastUpdateTime = FPlatformTime::Seconds();

		const FVector CurrentLoation = GetActorLocation();
		const FVector NewLocation = FVector(FMath::RandRange(-500, 4500), FMath::RandRange(-500, 4500), CurrentLoation.Z);

		SetActorLocation(NewLocation);
	}
}

// Called to bind functionality to input
void AFovCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AFovCharacter::OnTickFov(TSet<FVector2D>& NewHiddenSight, TSet<FVector2D>& NewViewingSight)
{
	NewHiddenSight.Empty();
	NewViewingSight.Empty();

	if (!FogOfWar.IsValid())
	{
		return;
	}

	const int32 SightBlockCount = FMath::RoundToInt(SightRange / FogOfWar->GetSightBlockSize());
	const FVector CurrentLocation = GetActorLocation();

	const int32 CurrentSightBlockRow = CurrentLocation.X / FogOfWar->GetSightBlockSize();
	const int32 CurrentSightBlockColumn = CurrentLocation.Y / FogOfWar->GetSightBlockSize();

	TSet<FVector2D> NewSightSet;
	FovCircle(CurrentSightBlockColumn, CurrentSightBlockRow, SightBlockCount, &NewSightSet);

	const TSet<FVector2D> NoChangeSight = NewSightSet.Intersect(SightInfoSet);
	NewHiddenSight = SightInfoSet.Difference(NoChangeSight);
	NewViewingSight = NewSightSet.Difference(NoChangeSight);
	SightInfoSet = NewSightSet;
}

bool AFovCharacter::CanSee(int32 SrcX, int32 SrcY, int32 TargetX, int32 TargetY)
{
	return true;
}

