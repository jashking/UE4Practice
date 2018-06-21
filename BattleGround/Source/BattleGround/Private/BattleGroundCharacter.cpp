// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleGroundCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Engine/World.h"

#include "BattleGroundWeapon.h"

// Sets default values
ABattleGroundCharacter::ABattleGroundCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComp->SetupAttachment(SpringArmComp);

	WeaponAttachSocketName = TEXT("WeaponSocket");
}

// Called when the game starts or when spawned
void ABattleGroundCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultFOV = CameraComp->FieldOfView;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	CurrentWeapon = GetWorld()->SpawnActor<ABattleGroundWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (CurrentWeapon)
	{
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	}
}

void ABattleGroundCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ABattleGroundCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ABattleGroundCharacter::BeginCrouch()
{
	Crouch();
}

void ABattleGroundCharacter::EndCrouch()
{
	UnCrouch();
}

void ABattleGroundCharacter::BeginZoom()
{
	bWantsZoom = true;
}

void ABattleGroundCharacter::EndZoom()
{
	bWantsZoom = false;
}

void ABattleGroundCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ABattleGroundCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

// Called every frame
void ABattleGroundCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float TargetFOV = bWantsZoom ? ZoomFOV : DefaultFOV;
	const float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomSpeed);

	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ABattleGroundCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABattleGroundCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABattleGroundCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ABattleGroundCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ABattleGroundCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &ABattleGroundCharacter::BeginCrouch);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &ABattleGroundCharacter::EndCrouch);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction(TEXT("Zoom"), IE_Pressed, this, &ABattleGroundCharacter::BeginZoom);
	PlayerInputComponent->BindAction(TEXT("Zoom"), IE_Released, this, &ABattleGroundCharacter::EndZoom);

	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ABattleGroundCharacter::StartFire);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &ABattleGroundCharacter::StopFire);
}

FVector ABattleGroundCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

