// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BattleGroundWeapon.h"
#include "BattleGroundProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEGROUND_API ABattleGroundProjectileWeapon : public ABattleGroundWeapon
{
	GENERATED_BODY()
	
protected:
	virtual void Fire() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProjectileWeapon")
	TSubclassOf<AActor> ProjectileClass;
};
