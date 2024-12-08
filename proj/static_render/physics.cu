#include "function/global_context.h"
#include "function/resource_manager/resource_manager.h"
#include "function/tool/fire_light_updater.h"
#include "function/type/vertex.h"
#include "physics.h"
#include <cstdio>
#include <cuda_runtime.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define get_bid() (blockIdx.z * gridDim.y * gridDim.x + blockIdx.y * gridDim.x + blockIdx.x)
#define get_tid() (get_bid() * (blockDim.x * blockDim.y * blockDim.z) + threadIdx.z * blockDim.x * blockDim.y + threadIdx.y * blockDim.x + threadIdx.x)

// int vertex_cnt = extBuffers["cube1"].size / sizeof(Vertex);
// scale<<<1, vertex_cnt, 0, streamToRun>>>((Vertex*)extBuffers["cube1"].ptr);
__global__ void scale(Vertex* vertexBuffer)
{
    auto tid = get_tid();

    auto& vertex = vertexBuffer[tid];

    float scale_factor = 1.0003f;
    if (abs(vertex.pos.x) > 0.6) {
        scale_factor = 0.8f;
    }

    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::scale(mat, glm::vec3(scale_factor));
    glm::vec4 temp(vertex.pos, 1);
    temp = mat * temp;
    vertex.pos.x = temp.x;
    vertex.pos.y = temp.y;
    vertex.pos.z = temp.z;
}

// const int threadsPerBlock = 64;
// const int numBlocks = 128 * 128 * 128 / threadsPerBlock;
// fill<<<numBlocks, threadsPerBlock, 0, streamToRun>>>(
//     extImages["smoke_field"].surface_object,
//     extImages["smoke_field"].extent,
//     extImages["smoke_field"].element_size);
__global__ void fill(cudaSurfaceObject_t surface_object, cudaExtent extent, size_t element_size)
{
    auto id = get_tid();
    int x = id / (extent.height * extent.depth);
    int y = (id % (extent.height * extent.depth)) / extent.depth;
    int z = id % extent.depth;

    if (x >= 30 && x < 60 && y >= 20 && y < 40 && z >= 10 && z < 20) {
        surf3Dwrite(50.0f, surface_object, x * element_size, y, z);
    } else {
        surf3Dwrite(1.f, surface_object, x * element_size, y, z);
    }
}

void PhysicsEngineUser::init(Configuration& config, GlobalContext* g_ctx)
{
    CudaEngine::init(config, g_ctx);
}

void PhysicsEngineUser::initExternalMem()
{
    //     for (auto& object : g_ctx->rm->objects) {
    // #ifdef _WIN64
    //         HANDLE handle = object.getVkVertexMemHandle();
    // #else
    //         int fd = object.getVkVertexMemHandle();
    // #endif
    //
    //         const auto& mesh = g_ctx->rm->meshes[object.mesh];
    //         size_t size = mesh.data.vertices.size() * sizeof(Vertex);
    //         CudaEngine::ExtBufferDesc buffer_desc = {
    // #ifdef _WIN64
    //             handle,
    // #else
    //             fd,
    // #endif
    //             size,
    //             object.name
    //         };
    //         this->importExtBuffer(buffer_desc); // add to extBuffers internally
    //     }

    for (int i = 0; i < g_ctx->rm->fields.fields.size(); i++) {
        auto& field = g_ctx->rm->fields.fields[i];
#ifdef _WIN64
        HANDLE handle = shared_data.fields.getVkFieldMemHandle(i);
#else
        int fd = g_ctx->rm->fields.getVkFieldMemHandle(i);
#endif
        CudaEngine::ExtImageDesc image_desc = {
#ifdef _WIN64
            handle,
#else
            fd,
#endif
            128 * 256 * 128 * sizeof(float),
            sizeof(float),
            128,
            256,
            128,
            field.name
        };
        this->importExtImage(image_desc); // add to extBuffers internally
    }
}

void PhysicsEngineUser::step()
{
    waitOnSemaphore(vkUpdateSemaphore);

    if (g_ctx->rm->fields.has_temperature) {
        g_ctx->rm->fields.lights_updater->updateFireLightData(
            extImages["fire_field"].surface_object,
            streamToRun,
            g_ctx->rm->fields.lights);
    }

    signalSemaphore(cuUpdateSemaphore);
}

void PhysicsEngineUser::cleanup()
{
    CudaEngine::cleanup();
}
