#include "fbxsdk.h"

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "opengl_wrappers.h"

int main()
{
    glfwInit();

    //window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1000, 500, "LearnOpenGL", NULL, NULL);

    if (window == nullptr)
    {
        std::cout << "Failed to create a GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //opengl
    int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (version == 0) {
        std::cout << "Failed to initialize OpenGL context\n";
        return -1;
    }
    printf("Loaded OpenGL\n");

    //loop
    float m_vertices[9] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    unsigned int m_indices[3] = {
        0, 1, 2
    };
    const char* vertex_shader_source =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;"

        "void main()"
        "{"
        "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
        "};";

    const char* fragment_shader_source =
        "#version 330 core\n"
        "out vec4 FragColor;"

        "void main()"
        "{"
        "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
        "}";
    VertexArray vao(m_vertices, 9, m_indices, 3);
    Program shader(vertex_shader_source, fragment_shader_source);
    while (true)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(window))
            break;

        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw(vao, shader);

        glfwSwapBuffers(window);
    }

    //close
    glfwTerminate();
    return 0;
}