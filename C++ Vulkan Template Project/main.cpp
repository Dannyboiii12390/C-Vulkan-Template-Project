#include <fstream>
#include <iostream>
#include <chrono>
#include "VulkanContext.h"

int main()
{

    //main bottleneck at the moment is loading the textures

    std::fstream fpsFile("fps_log.txt", std::ios::out);

    float totalTime = 0.0f;
    int frameCount = 0;
    float secondAccumulator = 0.0f;
    int framesThisSecond = 0;

    VulkanContext app;
    try
    {
        while (!app.getWindow().shouldClose())
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            glfwPollEvents();
            app.getInputHandler().update();
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

    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
