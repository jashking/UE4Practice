#pragma once

#include "CoreMinimal.h"

/** Eight-way directions. */
enum class EFovDirectionType
{
	FOV_EAST = 0,
	FOV_NORTHEAST,
	FOV_NORTH,
	FOV_NORTHWEST,
	FOV_WEST,
	FOV_SOUTHWEST,
	FOV_SOUTH,
	FOV_SOUTHEAST
};

/** Values for the shape setting. */
enum class EFovShapeType
{
	FOV_SHAPE_CIRCLE_PRECALCULATE,
	FOV_SHAPE_SQUARE,
	FOV_SHAPE_CIRCLE,
	FOV_SHAPE_OCTAGON
};

/** Values for the corner peek setting. */
enum EFovCornerPeekType
{
	FOV_CORNER_NOPEEK,
	FOV_CORNER_PEEK
};

/** Values for the opaque apply setting. */
enum EFovOpaqueApplyType
{
	FOV_OPAQUE_APPLY,
	FOV_OPAQUE_NOAPPLY
};

class IFovInterface
{
public:
	IFovInterface();
	virtual ~IFovInterface();

	/** Opacity test callback.
	 * Set the function used to test whether a map tile is opaque.
	 *
	 * \param settings Pointer to data structure containing settings.
	 * \param f The function called to test whether a map tile is opaque.
	 */
	virtual bool CanSee(int32 SrcX, int32 SrcY, int32 TargetX, int32 TargetY) = 0;

	/**
	 * Calculate a full circle field of view from a source at (x,y).
	 *
	 * \param settings Pointer to data structure containing settings.
	 * \param map Pointer to map data structure to be passed to callbacks.
	 * \param source Pointer to data structure holding source of light.
	 * \param source_x x-axis coordinate from which to start.
	 * \param source_y y-axis coordinate from which to start.
	 * \param radius Euclidean distance from (x,y) after which to stop.
	 */
	virtual void FovCircle(int32 InSourceX, int32 InSourceY, uint32 InRadius, TSet<FVector2D>* NewSight);

	/**
	 * Set the shape of the field of view.
	 *
	 * \param settings Pointer to data structure containing settings.
	 * \param value One of the following values, where R is the radius:
	 *
	 * - FOV_SHAPE_CIRCLE_PRECALCULATE \b (default): Limit the FOV to a
	 * circle with radius R by precalculating, which consumes more memory
	 * at the rate of 4*(R+2) bytes per R used in calls to fov_circle.
	 * Each radius is only calculated once so that it can be used again.
	 * Use fov_free() to free this precalculated data's memory.
	 *
	 * - FOV_SHAPE_CIRCLE: Limit the FOV to a circle with radius R by
	 * calculating on-the-fly.
	 *
	 * - FOV_SHAPE_OCTOGON: Limit the FOV to an octogon with maximum radius R.
	 *
	 * - FOV_SHAPE_SQUARE: Limit the FOV to an R*R square.
	 */
	void SetShape(EFovShapeType InShape)
	{
		Shape = InShape;
	}

	EFovShapeType GetShape() const
	{
		return Shape;
	}

	/**
	 * <em>NOT YET IMPLEMENTED</em>.
	 *
	 * Set whether sources will peek around corners.
	 *
	 * \param settings Pointer to data structure containing settings.
	 * \param value One of the following values:
	 *
	 * - FOV_CORNER_PEEK \b (default): Renders:
	 \verbatim
	 ........
	 ........
	 ........
	 ..@#
	 ...#
	 \endverbatim
	 * - FOV_CORNER_NOPEEK: Renders:
	 \verbatim
	 ......
	 .....
	 ....
	 ..@#
	 ...#
	 \endverbatim
	 */
	void SetCornerPeek(EFovCornerPeekType InCornerPeek)
	{
		CornerPeek = InCornerPeek;
	}

	EFovCornerPeekType GetCornerPeek() const
	{
		return CornerPeek;
	}

	/**
	 * Whether to call the apply callback on opaque tiles.
	 *
	 * \param settings Pointer to data structure containing settings.
	 * \param value One of the following values:
	 *
	 * - FOV_OPAQUE_APPLY \b (default): Call apply callback on opaque tiles.
	 * - FOV_OPAQUE_NOAPPLY: Do not call the apply callback on opaque tiles.
	 */
	void SetOpaqueApply(EFovOpaqueApplyType InApplyType)
	{
		OpaqueApply = InApplyType;
	}

	EFovOpaqueApplyType GetOpaqueApply() const
	{
		return OpaqueApply;
	}

protected:
	/** Shape setting. */
	EFovShapeType Shape;

	/** Whether to peek around corners. */
	EFovCornerPeekType CornerPeek;

	/** Whether to call apply on opaque tiles. */
	EFovOpaqueApplyType OpaqueApply;
};
