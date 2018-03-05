#include "FovObjectInterface.h"

void IFovObjectInterface::OnTickFov(TSet<FVector2D>& NewHiddenSight, TSet<FVector2D>& NewViewingSight)
{
	NewHiddenSight.Empty();
	NewViewingSight.Empty();
}
