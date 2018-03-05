#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"

#include "FovObjectInterface.generated.h"

UINTERFACE()
class FOGOFWARDEMO_API UFovObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class FOGOFWARDEMO_API IFovObjectInterface
{
	GENERATED_BODY()

public:

	virtual void OnTickFov(TSet<FVector2D>& NewHiddenSight, TSet<FVector2D>& NewViewingSight);

protected:

	TSet<FVector2D> SightInfoSet;
};
