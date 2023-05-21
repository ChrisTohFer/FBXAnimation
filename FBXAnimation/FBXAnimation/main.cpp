#include "fbx_wrapper.h"
#include "opengl_wrappers.h"

#include "vector3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <vector>

template<typename Vertex>
struct IndexedVertices
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

IndexedVertices<geom::Vector3> get_vertices_from_fbx(const FBXSceneWrapper& fbx_scene_wrapper)
{
    const FbxScene* scene = fbx_scene_wrapper.get_scene();
    _ASSERT(scene != nullptr);

    //get scene
    if (scene == nullptr)
    {
        return {};
    }

    //get root node
    auto* root_node = scene->GetRootNode();
    if (root_node == nullptr)
    {
        return {};
    }

    //get mesh
    int child_count = root_node->GetChildCount();
    const FbxMesh* mesh = nullptr;
    for (int i = 0; i < child_count; ++i)
    {
        auto* child = root_node->GetChild(i);
        mesh = child->GetMesh();
        if (mesh != nullptr)
        {
            break;
        }
    }
    if (mesh == nullptr)
    {
        return {};
    }

    IndexedVertices<geom::Vector3> vertices;
    
    //get vertices
    FbxVector4* control_points = mesh->GetControlPoints();
    int control_points_count = mesh->GetControlPointsCount();
    vertices.vertices.reserve(control_points_count);
    for (int i = 0; i < control_points_count; ++i)
    {
        auto& point = control_points[i];
        vertices.vertices.push_back(geom::Vector3{
            (float)point.mData[0],
            (float)point.mData[1],
            (float)point.mData[2]
        });
    }

    //get triangles
    int polygon_count = mesh->GetPolygonCount();
    for (int i = 0; i < polygon_count; ++i)
    {
        int num_polygon_vertices = mesh->GetPolygonSize(i);
        _ASSERT(num_polygon_vertices != 3);

        vertices.indices.push_back(mesh->GetPolygonVertex(i, 0));
        vertices.indices.push_back(mesh->GetPolygonVertex(i, 1));
        vertices.indices.push_back(mesh->GetPolygonVertex(i, 2));
    }

    return vertices;
}

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

    //setup draw info/assets
#if false
    FBXManagerWrapper fbx_manager;
    FBXSceneWrapper test = fbx_manager.load("../../assets/fbx/cubeman_v2.fbx");
    auto vertices = get_vertices_from_fbx(test);
    VertexArray vao(
        (float*)vertices.vertices.data(),
        vertices.vertices.size() * 3,
        vertices.indices.data(),
        vertices.indices.size());
#else
    float vertices[9] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    unsigned int indices[3] = {
        0, 1, 2
    };
    VertexArray vao(
        vertices,
        9,
        indices,
        3);
#endif

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
    Program shader(vertex_shader_source, fragment_shader_source);
    while (true)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(window))
            break;

        //glEnable(GL_DEPTH_TEST);

        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw(vao, shader);

        glfwSwapBuffers(window);
    }

    //close
    glfwTerminate();
    return 0;
}