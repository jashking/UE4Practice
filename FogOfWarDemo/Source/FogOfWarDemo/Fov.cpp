#include "Fov.h"

struct fov_private_data_type 
{
	IFovInterface* settings;
	int32 source_x;
	int32 source_y;
	uint32 radius;
	TSet<FVector2D>* NewSight;
};

class FBaseOctant
{
public:
	virtual void FovOctant(fov_private_data_type *data, int32 dx, float start_slope, float end_slope) = 0;

	float FovSlope(float dx, float dy)
	{
		if (dx <= -FLT_EPSILON || dx >= FLT_EPSILON)
		{
			return dy / dx;
		}
		else
		{
			return 0.0;
		}
	}

protected:
	int32 SignX;
	int32 SignY;
	bool bApplyEdge;
	bool bApplyDiag;
};

class FBaseOctantXY : public FBaseOctant
{
public:
	virtual void FovOctant(fov_private_data_type *data, int32 dx, float start_slope, float end_slope) override
	{
		IFovInterface* settings = data->settings;

		if (dx == 0)
		{
			FovOctant(data, dx + 1, start_slope, end_slope);
			return;
		}
		else if ((uint32)dx > data->radius)
		{
			return;
		}

		int32 dy0 = (int32)(0.5f + ((float)dx) * start_slope);
		int32 dy1 = (int32)(0.5f + ((float)dx) * end_slope);
		int32 x = data->source_x + SignX * dx;
		int32 y = data->source_y + SignY * dy0;

		if (!bApplyDiag && dy1 == dx)
		{
			/* We do diagonal lines on every second octant, so they don't get done twice. */
			--dy1;
		}

		uint32 h = 0;
		switch (settings->GetShape())
		{
		case EFovShapeType::FOV_SHAPE_CIRCLE_PRECALCULATE:
		case EFovShapeType::FOV_SHAPE_CIRCLE:
			h = (uint32)sqrtf((float)(data->radius * data->radius - dx * dx));
			break;
		case EFovShapeType::FOV_SHAPE_OCTAGON:
			h = (data->radius - dx) << 1;
			break;
		default:
			h = data->radius;
			break;
		};

		if ((uint32)dy1 > h)
		{
			if (h == 0)
			{
				return;
			}

			dy1 = (int32)h;
		}

		int32 prev_blocked = -1;
		float end_slope_next = 0.f;
		for (int32 dy = dy0; dy <= dy1; ++dy)
		{
			y = data->source_y + SignY * dy;

			if (!settings->CanSee(data->source_x, data->source_y, x, y))
			{
				if (settings->GetOpaqueApply() == EFovOpaqueApplyType::FOV_OPAQUE_APPLY && (bApplyEdge || dy > 0))
				{
					if (data->NewSight)
					{
						data->NewSight->Add(FVector2D(y, x));
					}
				}

				if (prev_blocked == 0)
				{
					end_slope_next = FovSlope((float)dx + 0.5f, (float)dy - 0.5f);
					FovOctant(data, dx + 1, start_slope, end_slope_next);
				}

				prev_blocked = 1;
			}
			else
			{
				if (bApplyEdge || dy > 0)
				{
					if (data->NewSight)
					{
						data->NewSight->Add(FVector2D(y, x));
					}
				}

				if (prev_blocked == 1)
				{
					start_slope = FovSlope((float)dx - 0.5f, (float)dy - 0.5f);
				}

				prev_blocked = 0;
			}
		}

		if (prev_blocked == 0)
		{
			FovOctant(data, dx + 1, start_slope, end_slope);
		}
	}
};

class FBaseOctantYX : public FBaseOctant
{
public:
	virtual void FovOctant(fov_private_data_type *data, int32 dx, float start_slope, float end_slope) override
	{
		IFovInterface* settings = data->settings;

		if (dx == 0)
		{
			FovOctant(data, dx + 1, start_slope, end_slope);
			return;
		}
		else if ((uint32)dx > data->radius)
		{
			return;
		}

		int32 dy0 = (int32)(0.5f + ((float)dx) * start_slope);
		int32 dy1 = (int32)(0.5f + ((float)dx) * end_slope);
		int32 y = data->source_y + SignX * dx;
		int32 x = data->source_x + SignY * dy0;

		if (!bApplyDiag && dy1 == dx)
		{
			/* We do diagonal lines on every second octant, so they don't get done twice. */
			--dy1;
		}

		uint32 h = 0;
		switch (settings->GetShape())
		{
		case EFovShapeType::FOV_SHAPE_CIRCLE_PRECALCULATE:
		case EFovShapeType::FOV_SHAPE_CIRCLE:
			h = (uint32)sqrtf((float)(data->radius * data->radius - dx * dx));
			break;
		case EFovShapeType::FOV_SHAPE_OCTAGON:
			h = (data->radius - dx) << 1;
			break;
		default:
			h = data->radius;
			break;
		};

		if ((uint32)dy1 > h)
		{
			if (h == 0)
			{
				return;
			}

			dy1 = (int32)h;
		}

		int32 prev_blocked = -1;
		float end_slope_next = 0.f;
		for (int32 dy = dy0; dy <= dy1; ++dy)
		{
			x = data->source_x + SignY * dy;

			if (!settings->CanSee(data->source_x, data->source_y, x, y))
			{
				if (settings->GetOpaqueApply() == EFovOpaqueApplyType::FOV_OPAQUE_APPLY && (bApplyEdge || dy > 0))
				{
					if (data->NewSight)
					{
						data->NewSight->Add(FVector2D(y, x));
					}
				}

				if (prev_blocked == 0)
				{
					end_slope_next = FovSlope((float)dx + 0.5f, (float)dy - 0.5f);
					FovOctant(data, dx + 1, start_slope, end_slope_next);
				}

				prev_blocked = 1;
			}
			else
			{
				if (bApplyEdge || dy > 0)
				{
					if (data->NewSight)
					{
						data->NewSight->Add(FVector2D(y, x));
					}
				}

				if (prev_blocked == 1)
				{
					start_slope = FovSlope((float)dx - 0.5f, (float)dy - 0.5f);
				}

				prev_blocked = 0;
			}
		}

		if (prev_blocked == 0)
		{
			FovOctant(data, dx + 1, start_slope, end_slope);
		}
	}
};

// FOV_DEFINE_OCTANT(+, +, x, y, p, p, n, true, true)
class FOctantPPN : public FBaseOctantXY
{
public:
	FOctantPPN()
	{
		SignX = 1;
		SignY = 1;
		bApplyEdge = true;
		bApplyDiag = true;
	}
};

// FOV_DEFINE_OCTANT(+, -, x, y, p, m, n, false, true)
class FOctantPMN : public FBaseOctantXY
{
public:
	FOctantPMN()
	{
		SignX = 1;
		SignY = -1;
		bApplyEdge = false;
		bApplyDiag = true;
	}
};

// FOV_DEFINE_OCTANT(-, +, x, y, m, p, n, true, true)
class FOctantMPN : public FBaseOctantXY
{
public:
	FOctantMPN()
	{
		SignX = -1;
		SignY = 1;
		bApplyEdge = true;
		bApplyDiag = true;
	}
};

// FOV_DEFINE_OCTANT(-, -, x, y, m, m, n, false, true)
class FOctantMMN : public FBaseOctantXY
{
public:
	FOctantMMN()
	{
		SignX = -1;
		SignY = -1;
		bApplyEdge = false;
		bApplyDiag = true;
	}
};

// FOV_DEFINE_OCTANT(+, +, y, x, p, p, y, true, false)
class FOctantPPY : public FBaseOctantYX
{
public:
	FOctantPPY()
	{
		SignX = 1;
		SignY = 1;
		bApplyEdge = true;
		bApplyDiag = false;
	}
};

// FOV_DEFINE_OCTANT(+, -, y, x, p, m, y, false, false)
class FOctantPMY : public FBaseOctantYX
{
public:
	FOctantPMY()
	{
		SignX = 1;
		SignY = -1;
		bApplyEdge = false;
		bApplyDiag = false;
	}
};

// FOV_DEFINE_OCTANT(-, +, y, x, m, p, y, true, false)
class FOctantMPY : public FBaseOctantYX
{
public:
	FOctantMPY()
	{
		SignX = -1;
		SignY = 1;
		bApplyEdge = true;
		bApplyDiag = false;
	}
};

// FOV_DEFINE_OCTANT(-, -, y, x, m, m, y, false, false)
class FOctantMMY : public FBaseOctantYX
{
public:
	FOctantMMY()
	{
		SignX = -1;
		SignY = -1;
		bApplyEdge = false;
		bApplyDiag = false;
	}
};

static FOctantPPN OctantPPN;
static FOctantPPY OctantPPY;
static FOctantPMN OctantPMN;
static FOctantPMY OctantPMY;
static FOctantMPN OctantMPN;
static FOctantMPY OctantMPY;
static FOctantMMN OctantMMN;
static FOctantMMY OctantMMY;

IFovInterface::IFovInterface()
	: Shape(EFovShapeType::FOV_SHAPE_CIRCLE)
	, CornerPeek(EFovCornerPeekType::FOV_CORNER_NOPEEK)
	, OpaqueApply(EFovOpaqueApplyType::FOV_OPAQUE_NOAPPLY)
{
}

IFovInterface::~IFovInterface()
{
}

void IFovInterface::FovCircle(int32 InSourceX, int32 InSourceY, uint32 InRadius, TSet<FVector2D>* NewSight)
{
	/*
	 * Octants are defined by (x,y,r) where:
	 *  x = [p]ositive or [n]egative x increment
	 *  y = [p]ositive or [n]egative y increment
	 *  r = [y]es or [n]o for reflecting on axis x = y
	 *
	 *   \pmy|ppy/
	 *    \  |  /
	 *     \ | /
	 *   mpn\|/ppn
	 *   ----@----
	 *   mmn/|\pmn
	 *     / | \
	 *    /  |  \
	 *   /mmy|mpy\
	 */

	fov_private_data_type Data;

	Data.settings = this;
	Data.source_x = InSourceX;
	Data.source_y = InSourceY;
	Data.radius = InRadius;
	Data.NewSight = NewSight;

	if (NewSight)
	{
		NewSight->Empty();

		NewSight->Add(FVector2D(InSourceY, InSourceX));
	}

	OctantPPN.FovOctant(&Data, 1, (float)0.0f, (float)1.0f);
	OctantPPY.FovOctant(&Data, 1, (float)0.0f, (float)1.0f);
	OctantPMN.FovOctant(&Data, 1, (float)0.0f, (float)1.0f);
	OctantPMY.FovOctant(&Data, 1, (float)0.0f, (float)1.0f);
	OctantMPN.FovOctant(&Data, 1, (float)0.0f, (float)1.0f);
	OctantMPY.FovOctant(&Data, 1, (float)0.0f, (float)1.0f);
	OctantMMN.FovOctant(&Data, 1, (float)0.0f, (float)1.0f);
	OctantMMY.FovOctant(&Data, 1, (float)0.0f, (float)1.0f);
}
