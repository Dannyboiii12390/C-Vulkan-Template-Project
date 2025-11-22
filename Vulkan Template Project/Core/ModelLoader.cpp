// PSEUDOCODE / PLAN (detailed):
// 1. Interpret width/depth as number of cells; produce (width+1)*(depth+1) vertices.
// 2. Use cellSize to space vertices in world units; center grid around origin using halfWidth/halfDepth.
// 3. Provide a height function heightAt(ix, iz) that returns y for grid coordinates (uses sin/cos scaled by world coords).
// 4. Generate vertices in integer grid coordinates (ix in [0..width], iz in [0..depth]):
//    - compute world x,z: wx = (ix - halfWidth) * cellSize; wz = (iz - halfDepth) * cellSize
//    - compute y = heightAt(ix, iz)
//    - set vertex.pos = (wx, y, wz)
//    - compute normal via finite differences (central where possible, forward/backward at edges):
//        dhdx = (heightAt(ix+1,iz)-heightAt(ix-1,iz)) / (2*cellSize)  (or forward/back)
//        dhdz = (heightAt(ix,iz+1)-heightAt(ix,iz-1)) / (2*cellSize)
//        normal = normalize(vec3(-dhdx, 1.0f, -dhdz))
//    - set vertex.texCoord = (ix/width, iz/depth) so full terrain maps to [0,1] UVs
//    - set vertex.color default (white) — optional when textured
// 5. Generate triangle indices for each cell (two triangles per cell):
//    for iz in [0..depth-1], for ix in [0..width-1]:
//      a = iz*(width+1) + ix
//      b = a + 1
//      c = a + (width+1)
//      d = c + 1
//      push (a, c, d) and (a, d, b) — consistent winding (CCW) for the render pipeline
// 6. Cast indices to uint16_t (existing Mesh API expects uint16_t). Validate vertex count <= 65535 in comments.
// 7. Create mesh with vertices and indices and return.
//
// Notes:
// - This prepares normals and UVs so a texture can be applied.
// - If the terrain can exceed 65535 vertices, the Mesh/Index type must be updated to 32-bit indices.

Mesh ModelLoader::createTerrain(VulkanContext& context, int width, int depth, float cellSize)
{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    // Ensure width/depth represent cell counts; vertices per row = width + 1
    if (width <= 0) width = 1;
    if (depth <= 0) depth = 1;

    int halfWidth = width / 2;
    int halfDepth = depth / 2;

    const int vertsX = width + 1;
    const int vertsZ = depth + 1;

    // Simple height function using world coordinates (can be replaced by heightmap)
    auto heightAt = [&](int ix, int iz) -> float {
        float wx = (static_cast<float>(ix) - static_cast<float>(halfWidth)) * cellSize;
        float wz = (static_cast<float>(iz) - static_cast<float>(halfDepth)) * cellSize;
        return std::sinf(wx) * std::cosf(wz); // current height function
    };

    // Reserve memory
    vertices.reserve(static_cast<size_t>(vertsX) * static_cast<size_t>(vertsZ));
    indices.reserve(static_cast<size_t>(width) * static_cast<size_t>(depth) * 6);

    // Generate vertices with positions, normals (computed later per-vertex using finite differences), texcoords and color.
    // We'll compute normals on the fly using neighboring height samples.
    for (int iz = 0; iz < vertsZ; ++iz)
    {
        for (int ix = 0; ix < vertsX; ++ix)
        {
            // world coordinates
            float wx = (static_cast<float>(ix) - static_cast<float>(halfWidth)) * cellSize;
            float wz = (static_cast<float>(iz) - static_cast<float>(halfDepth)) * cellSize;
            float y = heightAt(ix, iz);

            Engine::Vertex v{};
            v.pos = glm::vec3(wx, y, wz);

            // Compute finite-difference derivatives for normal
            // X derivative (dhdx)
            float hL = (ix > 0) ? heightAt(ix - 1, iz) : heightAt(ix, iz);
            float hR = (ix < vertsX - 1) ? heightAt(ix + 1, iz) : heightAt(ix, iz);
            float dhdx = (hR - hL) / ( ( (ix > 0 && ix < vertsX - 1) ? (2.0f * cellSize) : cellSize) );

            // Z derivative (dhdz)
            float hD = (iz > 0) ? heightAt(ix, iz - 1) : heightAt(ix, iz);
            float hU = (iz < vertsZ - 1) ? heightAt(ix, iz + 1) : heightAt(ix, iz);
            float dhdz = (hU - hD) / ( ( (iz > 0 && iz < vertsZ - 1) ? (2.0f * cellSize) : cellSize) );

            glm::vec3 n = glm::normalize(glm::vec3(-dhdx, 1.0f, -dhdz));
            v.normal = n;

            // Texture coordinates mapped over entire terrain (0..1). Ready for tiling if shader multiplies UVs.
            v.texCoord = glm::vec2(static_cast<float>(ix) / static_cast<float>(width),
                                   static_cast<float>(iz) / static_cast<float>(depth));

            // Default color (white) — texture will modulate this or replaced by sampled color.
            v.color = glm::vec3(1.0f, 1.0f, 1.0f);

            vertices.push_back(v);
        }
    }

    // Generate triangle indices (two triangles per quad cell)
    for (int iz = 0; iz < depth; ++iz)
    {
        for (int ix = 0; ix < width; ++ix)
        {
            uint32_t a = static_cast<uint32_t>(iz) * vertsX + ix;
            uint32_t b = a + 1;
            uint32_t c = a + vertsX;
            uint32_t d = c + 1;

            // Ensure indices fit into uint16_t for current Mesh implementation.
            // If the terrain exceeds 65535 vertices, Mesh must be updated to support 32-bit indices.
            indices.push_back(static_cast<uint16_t>(a));
            indices.push_back(static_cast<uint16_t>(c));
            indices.push_back(static_cast<uint16_t>(d));

            indices.push_back(static_cast<uint16_t>(a));
            indices.push_back(static_cast<uint16_t>(d));
            indices.push_back(static_cast<uint16_t>(b));
        }
    }

    Mesh mesh;
    mesh.create(context, std::move(vertices), std::move(indices));
    return mesh;
}