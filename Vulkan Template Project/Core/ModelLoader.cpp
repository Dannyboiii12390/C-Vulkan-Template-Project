// Pseudocode / Plan:
// 1. Build grid positions and texcoords exactly as before.
// 2. Use an index buffer of type uint16_t (the Mesh::create overload expects uint16_t indices).
// 3. When computing a,b,c,d for each quad convert the computed index to uint16_t.
// 4. Reserve and fill indices accordingly, and when using them to look up positions cast to uint32_t/size_t as needed.
// 5. Accumulate face normals and normalize them.
// 6. Build final Vertex list and compute simple tangent/binormal.
// 7. Call mesh.create(context, std::move(finalVertices), std::move(indices)) with uint16_t indices.
//
// Note: This replaces only the createTerrain implementation to fix the overload mismatch
// by ensuring indices are stored as uint16_t to match Mesh::create signature.

Mesh ModelLoader::createTerrain(VulkanContext& context, int totalWidth, int totalDepth, float cellSize, float UVsize)
{
    std::vector<Vertex> finalVertices;
    std::vector<uint16_t> indices; // use uint16_t to match Mesh::create signature

    // Validate inputs
    if (cellSize <= 0.0f) cellSize = 1.0f;
    if (totalWidth <= 0) totalWidth = 1;
    if (totalDepth <= 0) totalDepth = 1;

    // Number of cells
    int cellsX = std::max(1, static_cast<int>(std::floor(static_cast<float>(totalWidth) / cellSize)));
    int cellsZ = std::max(1, static_cast<int>(std::floor(static_cast<float>(totalDepth) / cellSize)));

    const int vertsX = cellsX + 1;
    const int vertsZ = cellsZ + 1;

    const float halfWidth = (cellsX * cellSize) * 0.5f;
    const float halfDepth = (cellsZ * cellSize) * 0.5f;

    auto heightAt = [&](int ix, int iz) -> float
    {
        float wx = (static_cast<float>(ix) * cellSize) - halfWidth;
        float wz = (static_cast<float>(iz) * cellSize) - halfDepth;
        return std::sinf(wx) * std::cosf(wz);
    };

    auto positionAt = [&](int ix, int iz) -> glm::vec3
    {
        float wx = (static_cast<float>(ix) * cellSize) - halfWidth;
        float wz = (static_cast<float>(iz) * cellSize) - halfDepth;
        float y = heightAt(ix, iz);
        return glm::vec3(wx, y, wz);
    };

    // Build grid of positions and texcoords
    std::vector<glm::vec3> positions(vertsX * vertsZ);
    std::vector<glm::vec2> texcoords(vertsX * vertsZ);
    for (int iz = 0; iz < vertsZ; ++iz)
    {
        for (int ix = 0; ix < vertsX; ++ix)
        {
            int idx = iz * vertsX + ix;
            positions[idx] = positionAt(ix, iz);
            texcoords[idx] = glm::vec2(static_cast<float>(ix) / static_cast<float>(cellsX),
                static_cast<float>(iz) / static_cast<float>(cellsZ)) * UVsize;
        }
    }

    // Build indices (two triangles per quad)
    indices.reserve(static_cast<size_t>(cellsX) * static_cast<size_t>(cellsZ) * 6);
    for (int iz = 0; iz < cellsZ; ++iz)
    {
        for (int ix = 0; ix < cellsX; ++ix)
        {
            // compute indices and cast to uint16_t
            uint16_t a = static_cast<uint16_t>(iz * vertsX + ix);
            uint16_t b = static_cast<uint16_t>(iz * vertsX + (ix + 1));
            uint16_t c = static_cast<uint16_t>((iz + 1) * vertsX + ix);
            uint16_t d = static_cast<uint16_t>((iz + 1) * vertsX + (ix + 1));
            // First triangle: a, c, d
            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(d);
            // Second triangle: a, d, b
            indices.push_back(a);
            indices.push_back(d);
            indices.push_back(b);
        }
    }

    // Compute per-vertex normals by accumulating face normals
    std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f));
    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        uint32_t ia = static_cast<uint32_t>(indices[i + 0]);
        uint32_t ib = static_cast<uint32_t>(indices[i + 1]);
        uint32_t ic = static_cast<uint32_t>(indices[i + 2]);
        glm::vec3 pa = positions[ia];
        glm::vec3 pb = positions[ib];
        glm::vec3 pc = positions[ic];
        glm::vec3 faceNormal = glm::normalize(glm::cross(pb - pa, pc - pa));
        normals[ia] += faceNormal;
        normals[ib] += faceNormal;
        normals[ic] += faceNormal;
    }
    for (size_t i = 0; i < normals.size(); ++i)
    {
        normals[i] = glm::normalize(normals[i]);
    }

    // Build final Vertex array (shared vertices)
    finalVertices.reserve(positions.size());
    for (size_t i = 0; i < positions.size(); ++i)
    {
        Vertex v{};
        v.pos = positions[i];
        v.normal = normals[i];
        v.texCoord = texcoords[i];
        v.color = glm::vec3(1.0f);
        // Provide a simple tangent/binormal; if you need accurate tangents, compute them explicitly.
        v.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        v.binormal = glm::normalize(glm::cross(v.normal, v.tangent));
        finalVertices.push_back(v);
    }

    Mesh mesh;
    // Use indexed mesh creation (indices are uint16_t to match Mesh::create)
    mesh.create(context, std::move(finalVertices), std::move(indices));
    return mesh;
}

Mesh ModelLoader::loadObjHomebrew(VulkanContext& context, const char* filepath, float UVsize)
{
    // Pseudocode:
    // 1. Open the file and read all lines.
    // 2. Parse vertex positions (v), texture coordinates (vt), normals (vn).
    // 3. Parse faces (f) and build indices.
    // 4. For each face, create Vertex objects (pos, normal, texCoord, color, tangent, binormal).
    // 5. Store vertices and indices, avoiding duplicates (vertex deduplication).
    // 6. Create Mesh using mesh.create(context, vertices, indices).

    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error(std::string("Failed to open OBJ file: ") + filepath);
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    struct Index { int v, vt, vn; bool operator<(const Index& o) const { return std::tie(v, vt, vn) < std::tie(o.v, o.vt, o.vn); } };
    std::map<Index, uint16_t> indexMap;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        if (prefix == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            positions.emplace_back(x, y, z);
        } else if (prefix == "vt") {
            float u, v;
            iss >> u >> v;
            texCoords.emplace_back(u * UVsize, v * UVsize);
        } else if (prefix == "vn") {
            float nx, ny, nz;
            iss >> nx >> ny >> nz;
            normals.emplace_back(nx, ny, nz);
        } else if (prefix == "f") {
            std::vector<Index> faceIndices;
            std::string vertStr;
            while (iss >> vertStr) {
                int v = 0, vt = 0, vn = 0;
                size_t p1 = vertStr.find('/');
                size_t p2 = vertStr.find('/', p1 + 1);
                v = std::stoi(vertStr.substr(0, p1)) - 1;
                if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1 + 1)
                    vt = std::stoi(vertStr.substr(p1 + 1, p2 - p1 - 1)) - 1;
                else
                    vt = -1;
                if (p2 != std::string::npos)
                    vn = std::stoi(vertStr.substr(p2 + 1)) - 1;
                else
                    vn = -1;
                faceIndices.push_back({ v, vt, vn });
            }
            // Triangulate polygons (assume convex)
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                Index idxs[3] = { faceIndices[0], faceIndices[i], faceIndices[i + 1] };
                for (int k = 0; k < 3; ++k) {
                    auto it = indexMap.find(idxs[k]);
                    if (it == indexMap.end()) {
                        Vertex vert{};
                        vert.pos = positions[idxs[k].v];
                        vert.texCoord = (idxs[k].vt >= 0 && idxs[k].vt < (int)texCoords.size()) ? texCoords[idxs[k].vt] : glm::vec2(0.0f);
                        vert.normal = (idxs[k].vn >= 0 && idxs[k].vn < (int)normals.size()) ? normals[idxs[k].vn] : glm::vec3(0.0f, 1.0f, 0.0f);
                        vert.color = glm::vec3(1.0f);
                        // Tangent/binormal: simple orthonormal basis
                        glm::vec3 up = glm::abs(vert.normal.y) < 0.999f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
                        vert.tangent = glm::normalize(glm::cross(up, vert.normal));
                        vert.binormal = glm::normalize(glm::cross(vert.normal, vert.tangent));
                        uint16_t idx = static_cast<uint16_t>(vertices.size());
                        vertices.push_back(vert);
                        indexMap[idxs[k]] = idx;
                        indices.push_back(idx);
                    } else {
                        indices.push_back(it->second);
                    }
                }
            }
        }
    }
    file.close();

    Mesh mesh;
    mesh.create(context, std::move(vertices), std::move(indices));
    return mesh;
}