
#pragma once

#include <string>

namespace Engine
{

    enum class FlattenMode
    {
        UseUVs,    // use vertex UV (vt) as X,Y (Z=0). Falls back to projection if vt missing.
        ProjectXY, // project vertex positions onto XY (z -> 0)
        ProjectXZ, // project positions onto XZ (y -> 0)
        ProjectYZ  // project positions onto YZ (x -> 0)
    };

    struct Key
    {
        int v, vt, vn;
        bool operator<(Key const& o) const
        {
            if (v != o.v) return v < o.v;
            if (vt != o.vt) return vt < o.vt;
            return vn < o.vn;
        }
    };

    // Flatten a 3D OBJ into a 2D OBJ.
    // - inputObjPath: path to source .obj
    // - outputObjPath: path to write flattened .obj
    // - mode: choose how to flatten
    // - flipV: when using UVs, optionally flip V (common UV convention differences)
    // Returns true on success, throws std::runtime_error on failure.
    void flattenOBJTo2D(const char* inputObjPath, const char* outputObjPath, FlattenMode mode = FlattenMode::UseUVs, bool flipV = true);

} // namespace Engine