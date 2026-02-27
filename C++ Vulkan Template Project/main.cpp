#include <fstream>
#include <iostream>
#include <chrono>
#include "VulkanContext.h"
#include <filesystem>

int main()
{

    std::fstream fpsFile("fps_log.txt", std::ios::out);

    float totalTime = 0.0f;
    int frameCount = 0;
    float secondAccumulator = 0.0f;
    int framesThisSecond = 0;

    VulkanContext app;
    
    while (!app.getWindow().shouldClose())
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        glfwPollEvents();
        app.updateInputHandler();
		app.updateDeltaTime();
        app.drawFrame();
        auto endTime = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        totalTime += elapsed;
        frameCount++;
        secondAccumulator += elapsed;
        framesThisSecond++;

        if (secondAccumulator >= 1000.0f)
        {
            float fps = framesThisSecond * 1000.0f / secondAccumulator;
            fpsFile << fps << "\n";
            secondAccumulator = 0.0f;
            framesThisSecond = 0;
        }
    }
    fpsFile << std::endl;
    float averageFPS = frameCount / (totalTime / 1000.0f);
    std::cout << "Average FPS: " << averageFPS << std::endl;

    vkDeviceWaitIdle(app.getDevice());
    app.cleanup();

    return EXIT_SUCCESS;
}

// BASIC FEATURES
/*
✔    Per vertex shading
✔    Per pixel shading
✔    Globe
✔    Ground plane
✔    Plants - grow rapidly after rain or snowfall, given the sun is shining.
✔    Objects (3/3)
✔    Model Loader
✔    Day/Night/Season Cycle - need to add season cycle
✔    Particle System
✔    Shadows - object rendered on the landscape - not actual shadows
✔    Cameras
✔    Controls
✔    Config file
*/

// ADVANCED FEATURES
/*
✔  [4 points] The globe can also be represented as a render pass whose behaviour with respect to appearance depends on the viewer position (inside vs. outside)
✔  [2 points] The configuration file can also be used to provide values for the initial direction of the sun/moon, the season lengths, or if/when environment effects happen.
✔  [4 points] Bump mapping to simulate elevated landscape on the inside of the globe
    [2 points] Displacement mapping to simulate elevated landscape on the inside of the globe
    [4 points] Environment mapping to be used for environment reflections on scene elements
✔  [4 points] Visualization of the sun and moon on the environment map for the inside of the globe with
    their apparent movement over time; if the viewer is outside the globe the sun/moon could be
    represented as (small) emissive spheres around the globe
✔  [6 points] Shadows - shadow mapping
✔  [4 points] Fake turbulent dust cloud, wafting through the desert at random, but originating in the
    centre of the globe and moving in a random direction
    [2 points] Statistics display for graphics and simulation information, toggled with key 's'
✔  [4 points] Statistics and control elements using IMGUI; for the control elements this is in addition to
    the mandatory keyboard-navigation features, for the statistics this would replace the simple statistics
    above
    [8 points] Deferred rendering, including the ability to switch the visualization for the various MRT
    buffers with key 'd'
    [4 points] High-dynamic range rendering and tone-mapped post-processing
    [4 points] Illuminating sparks intermixed with the fire particle system; the sparks are to be
    implemented as omnidirectional short-ranged light sources, which illuminate near objects within the
    scene.
*/
