#include "fbx_wrapper.h"
#include "opengl_wrappers.h"

#include "vector3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <chrono>
#include <iostream>
#include <vector>

struct SkinnedVertex
{
    geom::Vector3 pos;
    int skinned_bone_index;

    static void apply_attributes()
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 1, GL_INT, GL_FALSE, sizeof(SkinnedVertex), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
};

template<typename Vertex>
struct IndexedVertices
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

IndexedVertices<SkinnedVertex> get_vertices_from_fbx(const FBXSceneWrapper& fbx_scene_wrapper)
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

    IndexedVertices<SkinnedVertex> vertices;
    
    //face the mesh towards z with y upwards, then invert z
    //scene->GetGlobalSettings().getaxis
    auto transform = geom::create_x_rotation_matrix_44(3.14159f * 0.5f) * geom::create_scale_matrix_44({ 1.f, 1.f, -1.f });

    //get vertices
    const FbxSkin* skin = nullptr;
    if (mesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin) > 0)
    {
        skin = static_cast<const FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
    }

    FbxVector4* control_points = mesh->GetControlPoints();
    int control_points_count = mesh->GetControlPointsCount();
    vertices.vertices.reserve(control_points_count);
    for (int control_point_index = 0; control_point_index < control_points_count; ++control_point_index)
    {
        int skinned_index = 0;
        if (skin != nullptr)
        {
            int cluster_count = skin->GetClusterCount();
            double skinned_weight = 0.f;
            for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
            {
                const FbxCluster* cluster = skin->GetCluster(cluster_index);
                int control_point_indices_count = cluster->GetControlPointIndicesCount();
                int* control_point_indices = cluster->GetControlPointIndices();
                double* control_point_weights = cluster->GetControlPointWeights();
                for (int k = 0; k < control_point_indices_count; ++k)
                {
                    if (control_point_indices[k] == control_point_index)
                    {
                        if (control_point_weights[k] > skinned_weight)
                        {
                            skinned_weight = control_point_weights[k];
                            skinned_index = cluster_index;
                            break;
                        }
                    }
                }

            }
        }

        auto& point = control_points[control_point_index];
        vertices.vertices.push_back({ transform * geom::Vector3{
            (float)point.mData[0],
            (float)point.mData[1],
            (float)point.mData[2]},
            skinned_index
        });

    }

    //get triangles
    int polygon_count = mesh->GetPolygonCount();
    for (int i = 0; i < polygon_count; ++i)
    {
        int num_polygon_vertices = mesh->GetPolygonSize(i);
        _ASSERT(num_polygon_vertices == 3);

        vertices.indices.push_back(mesh->GetPolygonVertex(i, 0));
        vertices.indices.push_back(mesh->GetPolygonVertex(i, 1));
        vertices.indices.push_back(mesh->GetPolygonVertex(i, 2));
    }

    return vertices;
}

//vv TEMP vv

float g_timestep = 0.f;
bool g_up_press = false;
bool g_down_press = false;
bool g_left_press = false;
bool g_right_press = false;
bool g_w_press = false;
bool g_s_press = false;
bool g_a_press = false;
bool g_d_press = false;
bool g_space_press = false;
bool g_control_press = false;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool* key_flag = nullptr;
    switch (key)
    {
    case GLFW_KEY_UP: key_flag = &g_up_press; break;
    case GLFW_KEY_DOWN: key_flag = &g_down_press; break;
    case GLFW_KEY_LEFT: key_flag = &g_left_press; break;
    case GLFW_KEY_RIGHT: key_flag = &g_right_press; break;
    case GLFW_KEY_W: key_flag = &g_w_press; break;
    case GLFW_KEY_S: key_flag = &g_s_press; break;
    case GLFW_KEY_A: key_flag = &g_a_press; break;
    case GLFW_KEY_D: key_flag = &g_d_press; break;
    case GLFW_KEY_SPACE: key_flag = &g_space_press; break;
    case GLFW_KEY_LEFT_CONTROL: key_flag = &g_control_press; break;
    default: return;
    }

    if (action == GLFW_PRESS)
    {
        *key_flag = true;
    }
    else if (action == GLFW_RELEASE)
    {
        *key_flag = false;
    }
}

void resize_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_camera.aspect_ratio = (float)width / height;
}

//^^ TEMP ^^

int main()
{
    glfwInit();

    //window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1000, 500, "LearnOpenGL", NULL, NULL);
    g_camera.aspect_ratio = 1000.f / 500.f;

    if (window == nullptr)
    {
        std::cout << "Failed to create a GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, resize_callback);

    //opengl
    int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (version == 0) {
        std::cout << "Failed to initialize OpenGL context\n";
        return -1;
    }
    printf("Loaded OpenGL\n");

    //setup draw info/assets
#if true
    FBXManagerWrapper fbx_manager;
    FBXSceneWrapper test = fbx_manager.load("../../assets/fbx/cubeman_v2.fbx");
    auto vertices = get_vertices_from_fbx(test);
    VertexArray<SkinnedVertex> vao(
        VertexBuffer(std::move(vertices.vertices)),
        vertices.indices.data(),
        (int)vertices.indices.size());
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

        "uniform mat4 camera;"

        "void main()"
        "{"
        "gl_Position = camera * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
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
        using namespace std::chrono_literals;
        auto update_start_time = std::chrono::system_clock::now();

        glfwPollEvents();
        if (glfwWindowShouldClose(window))
            break;

        

        if (g_left_press) g_camera.rotation_euler.y -= 1.f * g_timestep;
        if (g_right_press) g_camera.rotation_euler.y += 1.f * g_timestep;
        if (g_up_press) g_camera.rotation_euler.x -= 1.f * g_timestep;
        if (g_down_press) g_camera.rotation_euler.x += 1.f * g_timestep;
        
        auto rotation_transform =
            geom::create_z_rotation_matrix_44(g_camera.rotation_euler.z) *
            geom::create_y_rotation_matrix_44(g_camera.rotation_euler.y) *
            geom::create_x_rotation_matrix_44(g_camera.rotation_euler.x);

        auto test_mat = rotation_transform * rotation_transform.inverse();
        auto t2 = geom::Matrix<2, 2>{ {0.5f, 0.f, 0.f, 0.5f} };
        auto test_mat2 = t2 * t2.inverse();

        if (g_w_press) g_camera.translation += rotation_transform * geom::Vector3::unit_z() * g_timestep;
        if (g_s_press) g_camera.translation -= rotation_transform * geom::Vector3::unit_z() * g_timestep;
        if (g_a_press) g_camera.translation -= rotation_transform * geom::Vector3::unit_x() * g_timestep;
        if (g_d_press) g_camera.translation += rotation_transform * geom::Vector3::unit_x() * g_timestep;
        if (g_space_press) g_camera.translation += rotation_transform * geom::Vector3::unit_y() * g_timestep;
        if (g_control_press) g_camera.translation -= rotation_transform * geom::Vector3::unit_y() * g_timestep;

        //glEnable(GL_DEPTH_TEST);

        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw(vao, shader);

        glfwSwapBuffers(window);

        auto duration = std::chrono::system_clock::now() - update_start_time;
        g_timestep = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() * 0.001f;
    }

    //close
    glfwTerminate();
    return 0;
}