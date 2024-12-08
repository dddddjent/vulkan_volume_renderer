#pragma once

#include "core/config/config.h"
#include "function/physics/cuda_engine.h"

class PhysicsEngineUser : public CudaEngine {
    virtual void initExternalMem() override;

public:
    virtual void init(Configuration& config, GlobalContext* g_ctx) override;
    virtual void step() override;
    virtual void cleanup() override;
};
