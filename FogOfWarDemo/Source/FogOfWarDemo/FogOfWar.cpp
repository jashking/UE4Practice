// Fill out your copyright notice in the Description page of Project Settings.

#include "FogOfWar.h"

#include "CanvasItem.h"
#include "EngineUtils.h"
#include "Engine/Canvas.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

#include "FovObjectInterface.h"

class FCanvasPixelItems : public FCanvasItem
{
public:
	FCanvasPixelItems(const TArray<FVector2D>& InPixels, const FLinearColor& InColor, int32 InTextureSize)
		: FCanvasItem(FVector2D::ZeroVector)
		, UV0(0.0f, 0.0f)
		, UV1(1.0f, 1.0f)
		, Texture(GWhiteTexture)
		, Pixels(InPixels)
		, TextureSize(InTextureSize)
	{
		SetColor(InColor);
	}

	virtual void Draw(FCanvas* InCanvas) override
	{
		// Draw the item
		if (Texture)
		{
			FLinearColor ActualColor = Color;
			ActualColor.A *= InCanvas->AlphaModulate;
			const FTexture* FinalTexture = Texture;
			FBatchedElements* BatchedElements = InCanvas->GetBatchedElements(FCanvas::ET_Triangle, BatchedElementParameters, FinalTexture, BlendMode);
			FHitProxyId HitProxyId = InCanvas->GetHitProxyId();

			for (const auto& Pixel : Pixels)
			{
				// Correct for Depth. This only works because we won't be applying a transform later--otherwise we'd have to adjust the transform instead.
				const float Left = Pixel.Y;
				const float Top = TextureSize - Pixel.X;
				const float Right = Pixel.Y + 1;
				const float Bottom = TextureSize - Pixel.X + 1;

				const int32 V00 = BatchedElements->AddVertex(FVector4(Left, Top, 0.0f, 1.f), FVector2D(UV0.X, UV0.Y), ActualColor, HitProxyId);
				const int32 V10 = BatchedElements->AddVertex(FVector4(Right, Top, 0.0f, 1.f), FVector2D(UV1.X, UV0.Y), ActualColor, HitProxyId);
				const int32 V01 = BatchedElements->AddVertex(FVector4(Left, Bottom, 0.0f, 1.f), FVector2D(UV0.X, UV1.Y), ActualColor, HitProxyId);
				const int32 V11 = BatchedElements->AddVertex(FVector4(Right, Bottom, 0.0f, 1.f), FVector2D(UV1.X, UV1.Y), ActualColor, HitProxyId);

				BatchedElements->AddTriangleExtensive(V00, V10, V11, BatchedElementParameters, FinalTexture, BlendMode);
				BatchedElements->AddTriangleExtensive(V00, V11, V01, BatchedElementParameters, FinalTexture, BlendMode);
			}
		}
	}

	/* Expose the functions defined in the base class. */
	using FCanvasItem::Draw;

	/* UV Coordinates 0 (Left/Top). */
	FVector2D UV0;

	/* UV Coordinates 0 (Right/Bottom). */
	FVector2D UV1;

	/* Texture to render. */
	const FTexture* Texture;

	const TArray<FVector2D>& Pixels;

	int32 TextureSize;
};

FLinearColor AFogOfWar::ViewingColor(0.f, 0.f, 0.f, 0.f);
FLinearColor AFogOfWar::HiddenColor(1.f, 0.f, 0.f, 0.f);

// Sets default values
AFogOfWar::AFogOfWar()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsPostProcessVolumeSpawnedDynamicly = false;
	bIsTextureAReadable = true;
	BlendValue = 1.f;
	TickInterval = 0.f;

	// Find default world fog material in content folder
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultFogOfWarMaterial(TEXT("/Game/FogOfWar/MI_FogOfWar"));
	FogOfWarMaterial = DefaultFogOfWarMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultFogOfWarCombineMaterial(TEXT("/Game/FogOfWar/M_FogBlend"));
	FogOfWarBlendMaterial = DefaultFogOfWarCombineMaterial.Object;
}

// Called when the game starts or when spawned
void AFogOfWar::BeginPlay()
{
	Super::BeginPlay();

	SetupRenderTarget();
	SetupWorldFog();
}

void AFogOfWar::BeginDestroy()
{
	FovObjectList.Empty();

	if (PostProcessVolume)
	{
		if (bIsPostProcessVolumeSpawnedDynamicly)
		{
			GetWorld()->DestroyActor(PostProcessVolume);
		}
		else
		{
			PostProcessVolume->Settings.RemoveBlendable(FogOfWarMaterialInst);
		}
	}

	PostProcessVolume = nullptr;

	UKismetRenderingLibrary::ReleaseRenderTarget2D(FogOfWarTexture);
	UKismetRenderingLibrary::ReleaseRenderTarget2D(FogOfWarTexture_A);
	UKismetRenderingLibrary::ReleaseRenderTarget2D(FogOfWarTexture_B);

	Super::BeginDestroy();
}

bool AFogOfWar::TickFov()
{
	bool bResult = false;
	TSet<FVector2D> NewHiddenSight;
	TSet<FVector2D> NewViewingSight;

	for (const auto& Object : FovObjectList)
	{
		if (Object)
		{
			Object->OnTickFov(NewHiddenSight, NewViewingSight);

			if (NewHiddenSight.Num() > 0 || NewViewingSight.Num() > 0)
			{
				UpdateSight(NewHiddenSight, NewViewingSight);
				bResult = true;
			}
		}
	}

	return bResult;
}

bool AFogOfWar::SetupRenderTarget()
{
	FogOfWarTexture = UKismetRenderingLibrary::CreateRenderTarget2D(this, FogOfWarTextureSize, FogOfWarTextureSize);
	if (FogOfWarTexture)
	{
		FogOfWarTexture->Filter = TF_Bilinear;
		UKismetRenderingLibrary::ClearRenderTarget2D(this, FogOfWarTexture, HiddenColor);
	}

	FogOfWarTexture_A = UKismetRenderingLibrary::CreateRenderTarget2D(this, FogOfWarTextureSize, FogOfWarTextureSize);
	if (FogOfWarTexture_A)
	{
		FogOfWarTexture_A->Filter = TF_Bilinear;
		UKismetRenderingLibrary::ClearRenderTarget2D(this, FogOfWarTexture_A, HiddenColor);
	}

	FogOfWarTexture_B = UKismetRenderingLibrary::CreateRenderTarget2D(this, FogOfWarTextureSize, FogOfWarTextureSize);
	if (FogOfWarTexture_B)
	{
		FogOfWarTexture_B->Filter = TF_Bilinear;
		UKismetRenderingLibrary::ClearRenderTarget2D(this, FogOfWarTexture_B, HiddenColor);
	}

	return (FogOfWarTexture != nullptr && FogOfWarTexture_A != nullptr && FogOfWarTexture_B != nullptr);
}

bool AFogOfWar::SetupWorldFog()
{
	if (!FogOfWarMaterial || !FogOfWarBlendMaterial)
	{
		return false;
	}

	// Find existing unbound post process volume in world.
	if (!PostProcessVolume)
	{
		for (TActorIterator<APostProcessVolume> Iter(GetWorld()); Iter; ++Iter)
		{
			if (Iter->bUnbound)
			{
				bIsPostProcessVolumeSpawnedDynamicly = false;
				PostProcessVolume = *Iter;
				break;
			}
		}
	}

	// No existing unbound post process volume, create a new one.
	if (!PostProcessVolume)
	{
		PostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>();
		PostProcessVolume->bUnbound = true;
		bIsPostProcessVolumeSpawnedDynamicly = true;
	}

	if (!PostProcessVolume)
	{
		return false;
	}

	FogOfWarMaterialInst = UMaterialInstanceDynamic::Create(FogOfWarMaterial, this);
	if (!FogOfWarMaterialInst)
	{
		return false;
	}

	FLinearColor FogBound(FogPosition.X, FogPosition.Y, FogOfWarTextureSize * BlockSize, FogOfWarTextureSize * BlockSize);
	FogOfWarMaterialInst->SetVectorParameterValue(TEXT("FogBound"), FogBound);
	FogOfWarMaterialInst->SetTextureParameterValue(TEXT("FogTexture"), GetWriteableRenderTarget());
	FogOfWarMaterialInst->SetScalarParameterValue(TEXT("TexutreInvSize"), 1.f / FogOfWarTextureSize);

	FogOfWarBlendMaterialInst = UMaterialInstanceDynamic::Create(FogOfWarBlendMaterial, this);
	if (!FogOfWarBlendMaterialInst)
	{
		return false;
	}

	FogOfWarBlendMaterialInst->SetTextureParameterValue(TEXT("OldFog"), GetReadableRenderTarget());
	FogOfWarBlendMaterialInst->SetTextureParameterValue(TEXT("NewFog"), FogOfWarTexture);
	FogOfWarBlendMaterialInst->SetScalarParameterValue(TEXT("FlipY"), RHINeedsToSwitchVerticalAxis(GShaderPlatformForFeatureLevel[GMaxRHIFeatureLevel]) ? 1 : 0);

	PostProcessVolume->AddOrUpdateBlendable(FogOfWarMaterialInst);

	return true;
}

bool AFogOfWar::UpdateFogOfWarTexture()
{
	UKismetRenderingLibrary::ClearRenderTarget2D(this, FogOfWarTexture, HiddenColor);

	UCanvas* Canvas;
	FVector2D Size;
	FDrawToRenderTargetContext RenderContext;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, FogOfWarTexture, Canvas, Size, RenderContext);

	TArray<FVector2D> Sights;
	SightMap.GetKeys(Sights);

	if (Sights.Num() > 0)
	{
		FCanvasPixelItems ItemViewing(Sights, ViewingColor, FogOfWarTextureSize);
		Canvas->DrawItem(ItemViewing);
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, RenderContext);

	BlendValue = 0.f;
	SwapTexture();

	return true;
}

bool AFogOfWar::BlendFogOfWarTexture(float DeltaTime /*= 0.f*/)
{
	if (!FogOfWarBlendMaterialInst || BlendValue >= 1.f)
	{
		return false;
	}

	const float BlendSpeed = DeltaTime / (BlendTime > 0.f ? BlendTime : 0.5f); // (1 / BlendTime * DeltaTime);
	BlendValue = FMath::Clamp(BlendValue + BlendSpeed, 0.f, 1.f);

	FogOfWarBlendMaterialInst->SetScalarParameterValue(TEXT("Blend"), BlendValue);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, GetWriteableRenderTarget(), FogOfWarBlendMaterialInst);

	return true;
}

class UTextureRenderTarget2D* AFogOfWar::GetReadableRenderTarget() const
{
	return bIsTextureAReadable ? FogOfWarTexture_A : FogOfWarTexture_B;
}

class UTextureRenderTarget2D* AFogOfWar::GetWriteableRenderTarget() const
{
	return bIsTextureAReadable ? FogOfWarTexture_B : FogOfWarTexture_A;
}

void AFogOfWar::SwapTexture()
{
	bIsTextureAReadable = !bIsTextureAReadable;

	if (FogOfWarMaterialInst)
	{
		FogOfWarMaterialInst->SetTextureParameterValue(TEXT("FogTexture"), GetWriteableRenderTarget());
	}

	if (FogOfWarBlendMaterialInst)
	{
		FogOfWarBlendMaterialInst->SetTextureParameterValue(TEXT("OldFog"), GetReadableRenderTarget());

		BlendFogOfWarTexture();
	}
}

// Called every frame
void AFogOfWar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickInterval += DeltaTime;
	if (TickInterval >= UpdateTime)
	{
		if (TickFov())
		{
			UpdateFogOfWarTexture();
		}

		TickInterval = 0.f;
	}

	BlendFogOfWarTexture(DeltaTime);
}

void AFogOfWar::AddFovObject(class IFovObjectInterface* InFovObject)
{
	FovObjectList.AddUnique(InFovObject);
}

void AFogOfWar::RemoveFovObject(class IFovObjectInterface* InFovObject)
{
	FovObjectList.Remove(InFovObject);
}

void AFogOfWar::UpdateSight(const TSet<FVector2D>& HiddenSight, const TSet<FVector2D>& ViewingSight)
{
	for (const auto& Hidden : HiddenSight)
	{
		int32* SightCount = SightMap.Find(Hidden);
		if (SightCount)
		{
			(*SightCount) -= 1;

			if (*SightCount <= 0)
			{
				SightMap.Remove(Hidden);
			}
		}
	}

	for (const auto& Viewing : ViewingSight)
	{
		int32* SightCount = SightMap.Find(Viewing);
		if (SightCount)
		{
			(*SightCount) += 1;
		}
		else
		{
			SightMap.Add(Viewing, 1);
		}
	}
}
