#pragma once

#include "Geometry.h"

namespace ShapeGenerator
{
    // TODO
    // [[nodiscard]]
    CPU_Geometry Sphere(float radius, int slices, int stacks);
    CPU_Geometry UnitCube();
};
