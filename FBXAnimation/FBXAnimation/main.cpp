#include "fbxsdk.h"

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "mesh.h"

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1000, 500, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (window == nullptr)
    {
        std::cout << "Failed to create a GLFW window\n";
        glfwTerminate();
        return -1;
    }

    int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (version == 0) {
        std::cout << "Failed to initialize OpenGL context\n";
        return -1;
    }
    
    // Successfully loaded OpenGL
    printf("Loaded OpenGL\n");

    Mesh mesh;
    while (true)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(window))
            break;

        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        mesh.draw();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}