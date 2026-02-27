// Append the following implementation to the end of Core/TextureBaker.cpp

#include <fstream>
#include <map>
#include <tuple>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include "Flattener.h"
#include <glm/glm.hpp>


namespace Engine
{

    struct ObjIndex
    {
        int v;  // position index (0-based), or -1
        int vt; // texcoord index (0-based), or -1
        int vn; // normal index (0-based), or -1
    };

    static inline ObjIndex parseFaceToken(const std::string& tok)
    {
        ObjIndex idx{ -1,-1,-1 };
        // formats: v, v/vt, v//vn, v/vt/vn
        size_t p1 = tok.find('/');
        if (p1 == std::string::npos)
        {
            idx.v = std::stoi(tok) - 1;
        }
        else
        {
            idx.v = std::stoi(tok.substr(0, p1)) - 1;
            size_t p2 = tok.find('/', p1 + 1);
            if (p2 == std::string::npos)
            {
                // v/vt
                std::string s = tok.substr(p1 + 1);
                if (!s.empty()) idx.vt = std::stoi(s) - 1;
            }
            else
            {
                // v//vn  or v/vt/vn
                if (p2 == p1 + 1)
                {
                    // v//vn
                    std::string s = tok.substr(p2 + 1);
                    if (!s.empty()) idx.vn = std::stoi(s) - 1;
                }
                else
                {
                    // v/vt/vn
                    std::string s1 = tok.substr(p1 + 1, p2 - p1 - 1);
                    if (!s1.empty()) idx.vt = std::stoi(s1) - 1;
                    std::string s2 = tok.substr(p2 + 1);
                    if (!s2.empty()) idx.vn = std::stoi(s2) - 1;
                }
            }
        }
        return idx;
    }

    void flattenOBJTo2D(const char* inputObjPath, const char* outputObjPath, FlattenMode mode, bool flipV)
    {
        // Read OBJ
        std::ifstream in(inputObjPath);
        if (!in.is_open()) throw std::runtime_error(std::string("Failed to open OBJ: ") + inputObjPath);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texcoords;
        std::vector<glm::vec3> normals;
        std::vector<std::vector<ObjIndex>> faces; // each face is list of indices

        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string tok;
            iss >> tok;
            if (tok == "v")
            {
                float x, y, z; iss >> x >> y >> z;
                positions.emplace_back(x, y, z);
            }
            else if (tok == "vt")
            {
                float u, v; iss >> u >> v;
                texcoords.emplace_back(u, v);
            }
            else if (tok == "vn")
            {
                float nx, ny, nz; iss >> nx >> ny >> nz;
                normals.emplace_back(nx, ny, nz);
            }
            else if (tok == "f")
            {
                std::vector<ObjIndex> face;
                std::string vertTok;
                while (iss >> vertTok)
                {
                    face.push_back(parseFaceToken(vertTok));
                }
                if (face.size() >= 3) faces.push_back(std::move(face));
            }
        }
        in.close();

        if (faces.empty()) throw std::runtime_error("No faces found in OBJ");

        // Create output vertex list and mapping (deduplicate by index triple or by vt if using UV mode)
        std::map<Key, int> indexMap;
        std::vector<glm::vec3> outPositions; // flattened 2D positions (Z=0)
        std::vector<std::pair<int, int>> outFaceIndices; // (faceId, vertexIndex) - not used; we will write faces on the fly

        auto make2Dpos_fromUV = [&](int vtIdx)->glm::vec3
            {
                if (vtIdx >= 0 && vtIdx < (int)texcoords.size())
                {
                    glm::vec2 uv = texcoords[vtIdx];
                    if (flipV) uv.y = 1.0f - uv.y;
                    return glm::vec3(uv.x, uv.y, 0.0f);
                }
                return glm::vec3(0.0f);
            };
        auto make2Dpos_fromPosition = [&](int vIdx, FlattenMode proj)->glm::vec3
            {
                glm::vec3 p = (vIdx >= 0 && vIdx < (int)positions.size()) ? positions[vIdx] : glm::vec3(0.0f);
                switch (proj)
                {
                case FlattenMode::ProjectXY: return glm::vec3(p.x, p.y, 0.0f);
                case FlattenMode::ProjectXZ: return glm::vec3(p.x, p.z, 0.0f);
                case FlattenMode::ProjectYZ: return glm::vec3(p.y, p.z, 0.0f);
                default: return glm::vec3(p.x, p.y, 0.0f);
                }
            };

        // Build mapping and new index arrays for faces
        std::vector<std::vector<int>> outFaces;
        outFaces.reserve(faces.size());

        for (const auto& face : faces)
        {
            std::vector<int> outFace;
            outFace.reserve(face.size());
            for (const auto& idx : face)
            {
                Key k;
                if (mode == FlattenMode::UseUVs)
                {
                    // Prefer vt when present; if no vt fall back to v projected
                    k.v = idx.v;
                    k.vt = idx.vt;
                    k.vn = idx.vn;
                }
                else
                {
                    // For projection modes, use only v index to dedupe
                    k.v = idx.v;
                    k.vt = -1;
                    k.vn = -1;
                }

                auto it = indexMap.find(k);
                if (it != indexMap.end())
                {
                    outFace.push_back(it->second);
                }
                else
                {
                    // create new output vertex
                    glm::vec3 pos2;
                    if (mode == FlattenMode::UseUVs)
                    {
                        if (idx.vt >= 0 && idx.vt < (int)texcoords.size())
                        {
                            pos2 = make2Dpos_fromUV(idx.vt);
                        }
                        else
                        {
                            // fallback to XY projection of original position
                            pos2 = make2Dpos_fromPosition(idx.v, FlattenMode::ProjectXY);
                        }
                    }
                    else
                    {
                        pos2 = make2Dpos_fromPosition(idx.v, mode);
                    }
                    int newIndex = static_cast<int>(outPositions.size()) + 1; // OBJ uses 1-based indices
                    indexMap.emplace(k, newIndex);
                    outPositions.push_back(pos2);
                    outFace.push_back(newIndex);
                }
            }
            outFaces.push_back(std::move(outFace));
        }

        // Write output OBJ (v + f). We produce v lines and faces referencing v indices.
        std::ofstream out(outputObjPath, std::ios::trunc);
        if (!out.is_open()) throw std::runtime_error(std::string("Failed to open output OBJ for write: ") + outputObjPath);

        out << "# Flattened OBJ produced from: " << inputObjPath << "\n";
        out << "# Mode: " << (mode == FlattenMode::UseUVs ? "UseUVs" : mode == FlattenMode::ProjectXY ? "ProjectXY" : mode == FlattenMode::ProjectXZ ? "ProjectXZ" : "ProjectYZ") << "\n";

        // write v lines (1-based order preserved by map insertion order)
        for (const auto& p : outPositions)
        {
            // keep Z = 0
            out << "v " << p.x << " " << p.y << " " << p.z << "\n";
        }

        // write faces
        for (const auto& f : outFaces)
        {
            out << "f";
            for (int idx : f)
            {
                out << " " << idx;
            }
            out << "\n";
        }

        out.close();
        return;
    }
}
