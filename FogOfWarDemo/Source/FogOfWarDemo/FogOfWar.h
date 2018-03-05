// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FogOfWar.generated.h"

UCLASS()
class FOGOFWARDEMO_API AFogOfWar : public AActor
{
	GENERATED_BODY()

public:
	static FLinearColor ViewingColor;
	static FLinearColor HiddenColor;

	// Sets default values for this actor's properties
	AFogOfWar();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void AddFovObject(class IFovObjectInterface* InInterface);
	void RemoveFovObject(class IFovObjectInterface* InInterface);

	int32 GetSightBlockSize() const
	{
		return BlockSize;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	bool TickFov();
	void UpdateSight(const TSet<FVector2D>& HiddenSight, const TSet<FVector2D>& ViewingSight);

	bool SetupRenderTarget();
	bool SetupWorldFog();

	bool UpdateFogOfWarTexture();
	bool BlendFogOfWarTexture(float DeltaTime = 0.f);

	class UTextureRenderTarget2D* GetReadableRenderTarget() const;
	class UTextureRenderTarget2D* GetWriteableRenderTarget() const;
	void SwapTexture();

protected:
	UPROPERTY(EditAnywhere, Category = "Fog Of War")
	class UMaterialInterface* FogOfWarMaterial;

	UPROPERTY(EditAnywhere, Category = "Fog Of War")
	class UMaterialInterface* FogOfWarBlendMaterial;

	UPROPERTY(EditAnywhere, Category = "Fog Of War")
	class APostProcessVolume* PostProcessVolume;

	UPROPERTY(EditAnywhere, Category = "Fog Of War", BlueprintReadOnly)
	int32 FogOfWarTextureSize = 512;

	UPROPERTY(EditAnywhere, Category = "Fog Of War", BlueprintReadOnly)
	FVector2D FogPosition = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Fog Of War", BlueprintReadOnly)
	float BlendTime = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Fog Of War", BlueprintReadOnly)
	float UpdateTime = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Fog Of War", BlueprintReadOnly)
	int32 BlockSize = 64;

private:
	// Instance of the fog post process material
	UPROPERTY(Transient)
	class UMaterialInstanceDynamic* FogOfWarMaterialInst;

	UPROPERTY(Transient)
	class UMaterialInstanceDynamic* FogOfWarBlendMaterialInst;

	UPROPERTY(Transient)
	class UTextureRenderTarget2D* FogOfWarTexture;

	UPROPERTY(Transient)
	class UTextureRenderTarget2D* FogOfWarTexture_A;

	UPROPERTY(Transient)
	class UTextureRenderTarget2D* FogOfWarTexture_B;

	bool bIsPostProcessVolumeSpawnedDynamicly;
	bool bIsTextureAReadable;
	float BlendValue;
	float TickInterval;

	TMap<FVector2D, int32> SightMap;

	TArray<class IFovObjectInterface*> FovObjectList;
};
