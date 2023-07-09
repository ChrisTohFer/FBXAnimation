#include "fbx_wrapper.h"

#include "maths/vector3.h"

#include "graphics/camera.h"
#include "graphics/core_shaders.h"

#include "animation/anim_pose.h"

#include "file/file_scanner.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui.h"

#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>


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

    //set up imgui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    //setup draw info/assets
    auto fbx_files = file::fbx_paths();
    FBXManagerWrapper fbx_manager;
    struct Character
    {
        FbxFileContent file_content;
        graphics::VertexArray<SkinnedVertex> vao;
    };
    std::vector<Character> characters;
    for (auto& fbx_file : fbx_files)
    {
        auto filepath = fbx_file.string();
        auto file_content = fbx_manager.load_file_content(filepath.c_str());
        auto vao = graphics::VertexArray<SkinnedVertex>(
            graphics::VertexBuffer(file_content.vertices),
            file_content.indices.data(),
            (int)file_content.indices.size());
        characters.push_back({ std::move(file_content), std::move(vao) });
    }

    //set up shaders
    graphics::UnskinnedMeshShader<SkinnedVertex> unskinned_shader;
    graphics::SkinnedMeshShader<SkinnedVertex> skinned_shader;
    graphics::DebugShader debug_shader;

    auto draw_skeleton = [&](
        const anim::Skeleton& skeleton,
        const std::vector<geom::Matrix44>& matrices,
        const geom::Matrix44& world)
    {
        _ASSERT(skeleton.bones.size() == matrices.size());
        for (int i = 0; i < matrices.size(); ++i)
        {
            const auto& bone = skeleton.bones[i];
            if (bone.parent_index == -1)
            {
                continue;
            }
            debug_shader.draw_line(
                g_camera, 
                world * matrices[i].translation(),
                world * matrices[bone.parent_index].translation());
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

        //openGL housekeeping
        //glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //ImGui Start frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

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

        //evaluate animation
        static float s_time = 0.f;
        s_time += g_timestep;

        struct Instance
        {
            enum Type
            {
                SkinnedMesh,
                UnskinnedMesh,
                SkinnedPose,
                RefPose,
            };
            Type type = RefPose;
            int mesh_index = 0;
            int anim_index = 0;
            geom::Vector3 translation = geom::Vector3::zero();
            geom::Vector3 euler = geom::Vector3::zero();
            geom::Vector3 scale = geom::Vector3::one();

            geom::Vector3 anim_mod_translation = geom::Vector3::zero();
            geom::Vector3 anim_mod_euler = geom::Vector3::zero();

            int id = next_id();
            static int next_id() { static int id = 0; return id++; }
        };
        static std::vector<Instance> s_instances;

        ImGui::Begin("Instances");
        if (ImGui::Button("Add"))
        {
            s_instances.push_back({});
        }
        int to_delete = -1;
        for (int i = 0; i < s_instances.size(); ++i)
        {
            auto& instance = s_instances[i];
            if (instance.mesh_index >= characters.size())
            {
                continue;
            }
            auto& character = characters[instance.mesh_index];

            //draw the instance
            auto world =
                geom::create_translation_matrix_44(instance.translation) *
                geom::create_z_rotation_matrix_44(instance.euler.z * geom::PI / 180.f) *
                geom::create_y_rotation_matrix_44(instance.euler.y * geom::PI / 180.f) *
                geom::create_x_rotation_matrix_44(instance.euler.x * geom::PI / 180.f) *
                geom::create_scale_matrix_44(instance.scale);

            auto create_matrix_stack = [&]()
            {
                std::vector<geom::Matrix44> mat_stack;
                if (character.file_content.animations.size() > instance.anim_index)
                {
                    mat_stack = character.file_content.animations[instance.anim_index].animation.get_pose(s_time, true).get_matrix_stack();
                }
                else
                {
                    mat_stack.resize(character.file_content.skeleton->bones.size(), geom::Matrix44::identity());
                }

                return mat_stack;
            };

            switch (instance.type)
            {
            case Instance::SkinnedMesh:
            {
                skinned_shader.draw(character.vao, g_camera.calculate_camera_matrix(), world, create_matrix_stack(), character.file_content.skeleton->inv_matrix_stack);
                break;
            }
            case Instance::UnskinnedMesh:
                unskinned_shader.draw(character.vao, g_camera.calculate_camera_matrix(), world);
                break;
            case Instance::SkinnedPose:
                draw_skeleton(*character.file_content.skeleton, create_matrix_stack(), world);
                break;
            case Instance::RefPose:
                draw_skeleton(*character.file_content.skeleton, character.file_content.skeleton->matrix_stack(), world);
                break;
            }

            //add edit details to imgui window
            char label[64];
            sprintf_s(label, "%d", instance.id);
            if(ImGui::CollapsingHeader(label))
            {
                ImGui::PushID(i);
                if (ImGui::Button("delete"))
                {
                    to_delete = i;
                }
                int type_int = (int)instance.type;
                ImGui::InputInt("Type", &type_int);
                instance.type = (Instance::Type)type_int;
                ImGui::InputInt("Mesh", &instance.mesh_index);
                ImGui::InputInt("Anim", &instance.anim_index);
                ImGui::DragFloat3("Position", &instance.translation.x, 0.2f);
                ImGui::DragFloat3("Rotation", &instance.euler.x, 5.f);
                ImGui::DragFloat3("Scale", &instance.scale.x, 0.01f);

                ImGui::Separator();

                ImGui::DragFloat3("Anim_Position", &instance.anim_mod_translation.x, 0.2f);
                ImGui::DragFloat3("Anim_Rotation", &instance.anim_mod_euler.x, 5.f);


                ImGui::PopID();
            }
        }
        ImGui::End();
        if (to_delete != -1)
        {
            s_instances.erase(s_instances.begin() + to_delete);
        }

        //ImGui end frame
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //flip buffer
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

    //ImGui End
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    //close
    glfwTerminate();
    return 0;
}