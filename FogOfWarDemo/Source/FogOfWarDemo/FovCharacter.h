// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "FovObjectInterface.h"
#include "FogOfWar.h"
#include "Fov.h"

#include "FovCharacter.generated.h"

UCLASS()
class FOGOFWARDEMO_API AFovCharacter : public ACharacter, public IFovObjectInterface, public IFovInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFovCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Fog Of War", BlueprintReadOnly)
	int32 SightRange = 1000;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnTickFov(TSet<FVector2D>& NewHiddenSight, TSet<FVector2D>& NewViewingSight) override;

	virtual bool CanSee(int32 SrcX, int32 SrcY, int32 TargetX, int32 TargetY) override;

private:
	TWeakObjectPtr<AFogOfWar> FogOfWar;
};
