// Loads the original embedded capsule model

#include "render/model.hpp"
#include "raymath.h"
#include "rlgl.h"

#include <cstring>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "external/tinyobj_loader_c.h"
#undef TINYOBJ_LOADER_C_IMPLEMENTATION

namespace bvhview
{
#include "render/capsule_obj.inc"
static Model LoadOBJFromMemory(const char* fileText)
{
    Model model = {0};

    tinyobj_attrib_t attrib = {0};
    tinyobj_shape_t* meshes = NULL;
    unsigned int meshCount = 0;

    tinyobj_material_t* materials = NULL;
    unsigned int materialCount = 0;

    if (fileText != NULL)
    {
        unsigned int dataSize = (unsigned int)strlen(fileText);

        unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
        tinyobj_parse_obj(&attrib, &meshes, &meshCount, &materials, &materialCount, fileText, dataSize, flags);

        model.meshCount = 1;
        model.meshes = (Mesh*)RL_CALLOC(model.meshCount, sizeof(Mesh));
        model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));

        int* matFaces = (int*)RL_CALLOC(model.meshCount, sizeof(int));
        matFaces[0] = attrib.num_faces;

        int* vCount = (int*)RL_CALLOC(model.meshCount, sizeof(int));
        int* vtCount = (int*)RL_CALLOC(model.meshCount, sizeof(int));
        int* vnCount = (int*)RL_CALLOC(model.meshCount, sizeof(int));
        int* faceCount = (int*)RL_CALLOC(model.meshCount, sizeof(int));

        for (int mi = 0; mi < model.meshCount; mi++)
        {
            model.meshes[mi].vertexCount = matFaces[mi] * 3;
            model.meshes[mi].triangleCount = matFaces[mi];
            model.meshes[mi].vertices = (float*)RL_CALLOC(model.meshes[mi].vertexCount * 3, sizeof(float));
            model.meshes[mi].texcoords = (float*)RL_CALLOC(model.meshes[mi].vertexCount * 2, sizeof(float));
            model.meshes[mi].normals = (float*)RL_CALLOC(model.meshes[mi].vertexCount * 3, sizeof(float));
            model.meshMaterial[mi] = mi;
        }

        for (unsigned int af = 0; af < attrib.num_faces; af++)
        {
            int mm = attrib.material_ids[af];
            if (mm == -1)
            {
                mm = 0;
            }

            tinyobj_vertex_index_t idx0 = attrib.faces[3 * af + 0];
            tinyobj_vertex_index_t idx1 = attrib.faces[3 * af + 1];
            tinyobj_vertex_index_t idx2 = attrib.faces[3 * af + 2];

            for (int v = 0; v < 3; v++)
            {
                model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx0.v_idx * 3 + v];
            }
            vCount[mm] += 3;
            for (int v = 0; v < 3; v++)
            {
                model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx1.v_idx * 3 + v];
            }
            vCount[mm] += 3;
            for (int v = 0; v < 3; v++)
            {
                model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx2.v_idx * 3 + v];
            }
            vCount[mm] += 3;

            if (attrib.num_texcoords > 0)
            {

                model.meshes[mm].texcoords[vtCount[mm] + 0] = attrib.texcoords[idx0.vt_idx * 2 + 0];
                model.meshes[mm].texcoords[vtCount[mm] + 1] = 1.0f - attrib.texcoords[idx0.vt_idx * 2 + 1];
                vtCount[mm] += 2;
                model.meshes[mm].texcoords[vtCount[mm] + 0] = attrib.texcoords[idx1.vt_idx * 2 + 0];
                model.meshes[mm].texcoords[vtCount[mm] + 1] = 1.0f - attrib.texcoords[idx1.vt_idx * 2 + 1];
                vtCount[mm] += 2;
                model.meshes[mm].texcoords[vtCount[mm] + 0] = attrib.texcoords[idx2.vt_idx * 2 + 0];
                model.meshes[mm].texcoords[vtCount[mm] + 1] = 1.0f - attrib.texcoords[idx2.vt_idx * 2 + 1];
                vtCount[mm] += 2;
            }

            if (attrib.num_normals > 0)
            {

                for (int v = 0; v < 3; v++)
                {
                    model.meshes[mm].normals[vnCount[mm] + v] = attrib.normals[idx0.vn_idx * 3 + v];
                }
                vnCount[mm] += 3;
                for (int v = 0; v < 3; v++)
                {
                    model.meshes[mm].normals[vnCount[mm] + v] = attrib.normals[idx1.vn_idx * 3 + v];
                }
                vnCount[mm] += 3;
                for (int v = 0; v < 3; v++)
                {
                    model.meshes[mm].normals[vnCount[mm] + v] = attrib.normals[idx2.vn_idx * 3 + v];
                }
                vnCount[mm] += 3;
            }
        }

        model.materialCount = 1;
        model.materials = (Material*)RL_CALLOC(model.materialCount, sizeof(Material));
        model.materials[0] = LoadMaterialDefault();

        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(meshes, meshCount);
        tinyobj_materials_free(materials, materialCount);

        RL_FREE(matFaces);
        RL_FREE(vCount);
        RL_FREE(vtCount);
        RL_FREE(vnCount);
        RL_FREE(faceCount);
    }

    model.transform = MatrixIdentity();

    for (int i = 0; i < model.meshCount; i++)
    {
        UploadMesh(&model.meshes[i], false);
    }

    return model;
}
Model LoadCapsuleModel()
{
    return LoadOBJFromMemory(capsuleOBJ);
}
}
