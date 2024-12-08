#include "engine.h"
#include "core/tool/logger.h"
#include <GLFW/glfw3.h>

void Engine::init(Configuration& config, RenderEngine* render_engine, UIEngine* ui_engine, PhysicsEngine* physics_engine)
{
    logger.init(config);
    INFO_FILE("Initializing the engine...");

    this->render_engine = render_engine;
    this->ui_engine = ui_engine;
    this->physics_engine = physics_engine;

    render_engine->init_core(config);
    window = render_engine->getGLFWWindow();

    g_ctx.init(config, window);

    render_engine->init_render(config, &g_ctx, ui_engine->getDrawUIFunction());

    physics_engine->init(config, &g_ctx);

    ui_engine->init(config, render_engine->toUI()); // get renderpass from render graph

    INFO_FILE("Engineinitialized");
}

void Engine::update_frame_time()
{
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - currentTime).count();
    g_ctx.frame_time = deltaTime;

    currentTime = std::chrono::high_resolution_clock::now();
}

void Engine::run()
{
    while (!glfwWindowShouldClose(window)) {
        INFO_FILE("Engine loop started");

        glfwPollEvents();

        update_frame_time();

        ui_engine->handleInput();
        render_engine->render();
        physics_engine->step();

        INFO_FILE("Engine loop ended");

        g_ctx.currentFrame++;
    }
}

void Engine::cleanup()
{
    INFO_FILE("Engine cleanup started");

    render_engine->sync();
    physics_engine->sync();

    physics_engine->cleanup();
    ui_engine->cleanup();
    render_engine->cleanup();
    g_ctx.cleanup();

    INFO_FILE("Engine cleanup ended");
}
