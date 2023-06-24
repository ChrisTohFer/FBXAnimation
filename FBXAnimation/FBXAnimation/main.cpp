#include "fbx_wrapper.h"

#include "maths/vector3.h"

#include "graphics/camera.h"
#include "graphics/core_shaders.h"

#include "anim_pose.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <chrono>
#include <iostream>
#include <vector>


//vv TEMP vv

graphics::Camera g_camera;

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
bool g_tab_press = false;
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
    case GLFW_KEY_TAB: key_flag = &g_tab_press; break;
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
    int framebuffer_x, framebuffer_y;
    glfwGetFramebufferSize(window, &framebuffer_x, &framebuffer_y);
    glViewport(0, 0, framebuffer_x, framebuffer_y);
    g_camera.aspect_ratio = (float)framebuffer_x / framebuffer_y;
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
    FBXManagerWrapper fbx_manager;
    FbxFileContent cubeman = fbx_manager.load_file_content("../../assets/fbx/cubeman_v2.fbx");
    graphics::VertexArray<SkinnedVertex> vao(
        graphics::VertexBuffer(cubeman.vertices),
        cubeman.indices.data(),
        (int)cubeman.indices.size());

    int anim_index = 0;

    graphics::UnskinnedMeshShader<SkinnedVertex> unskinned_shader;
    graphics::SkinnedMeshShader<SkinnedVertex> skinned_shader;
    graphics::DebugShader debug_shader;

    auto draw_skeleton = [&](const anim::Skeleton& skeleton, const std::vector<geom::Matrix44>& matrices)
    {
        _ASSERT(skeleton.bones.size() == matrices.size());
        for (int i = 0; i < matrices.size(); ++i)
        {
            const auto& bone = skeleton.bones[i];
            if (bone.parent_index == -1)
            {
                continue;
            }
            debug_shader.draw_line(g_camera, matrices[i].translation(), matrices[bone.parent_index].translation());
        }
    };

    while (true)
    {
        //timing start
        using namespace std::chrono_literals;
        auto update_start_time = std::chrono::system_clock::now();

        //window events
        glfwPollEvents();
        if (glfwWindowShouldClose(window))
            break;

        //camera movement
        if (g_left_press) g_camera.rotation_euler.y -= 1.f * g_timestep;
        if (g_right_press) g_camera.rotation_euler.y += 1.f * g_timestep;
        if (g_up_press) g_camera.rotation_euler.x -= 1.f * g_timestep;
        if (g_down_press) g_camera.rotation_euler.x += 1.f * g_timestep;
        
        auto rotation_transform =
            geom::create_z_rotation_matrix_44(g_camera.rotation_euler.z) *
            geom::create_y_rotation_matrix_44(g_camera.rotation_euler.y) *
            geom::create_x_rotation_matrix_44(g_camera.rotation_euler.x);

        if (g_w_press) g_camera.translation += rotation_transform * geom::Vector3::unit_z() * g_timestep;
        if (g_s_press) g_camera.translation -= rotation_transform * geom::Vector3::unit_z() * g_timestep;
        if (g_a_press) g_camera.translation -= rotation_transform * geom::Vector3::unit_x() * g_timestep;
        if (g_d_press) g_camera.translation += rotation_transform * geom::Vector3::unit_x() * g_timestep;
        if (g_space_press) g_camera.translation += rotation_transform * geom::Vector3::unit_y() * g_timestep;
        if (g_control_press) g_camera.translation -= rotation_transform * geom::Vector3::unit_y() * g_timestep;


        if (g_tab_press) anim_index = (anim_index + 1) % cubeman.animations.size();
        g_tab_press = false;

        //openGL housekeeping
        //glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //evaluate animation
        static float s_time = 0.f;
        s_time += g_timestep;
        std::vector<geom::Matrix44> mat_stack;

        if (cubeman.animations.size() > anim_index)
        {
            mat_stack = cubeman.animations[anim_index].animation.get_pose(s_time, true).get_matrix_stack();
        }

        //identity matrix stack for testing
        std::vector<geom::Matrix44> mat_stack_i(20, geom::Matrix44::identity());

        //draw
        //debug_shader.draw_line(g_camera, geom::Vector3::zero(), geom::Vector3::one());
        //unskinned_shader.draw(vao, g_camera.calculate_camera_matrix(), geom::create_translation_matrix_44({ 0.f,0.f,0.f }));
        skinned_shader.draw(vao, g_camera.calculate_camera_matrix(), geom::create_translation_matrix_44({ 0.f,0.f,0.f }), mat_stack, cubeman.skeleton->inv_matrix_stack);
        draw_skeleton(*cubeman.skeleton, mat_stack);


        glfwSwapBuffers(window);

        //check for errors
        auto err_code = glGetError();
        if (err_code != 0)
        {
            std::cout << "err_code = " << err_code << "\n";
            err_code = 0;
        }
        
        //timing end
        auto duration = std::chrono::system_clock::now() - update_start_time;
        g_timestep = (float) std::chrono::duration_cast<std::chrono::microseconds>(duration).count() * 0.000001f;
    }

    //close
    glfwTerminate();
    return 0;
}