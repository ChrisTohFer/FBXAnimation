#include "fbx_wrapper.h"
#include "opengl_wrappers.h"

#include "vector3.h"
#include "anim_pose.h"

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
        glVertexAttribIPointer(1, 1, GL_INT, sizeof(SkinnedVertex), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
};

template<typename Vertex>
struct IndexedVertices
{
    struct NamedAnim
    {
        std::string name;
        anim::Animation animation;
    };
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::unique_ptr<anim::Skeleton> skeleton;
    std::vector<NamedAnim> animations;
};

void process_armature_node(FbxNode& node, anim::Skeleton& skeleton, int parent_index, std::vector<FbxNode*>& nodes_out)
{
    std::cout << node.GetName() << "\n";
    int index = (int)skeleton.bones.size();
    nodes_out.push_back(&node);

    anim::Skeleton::Bone bone;
    auto global_translation = node.EvaluateGlobalTransform().GetT();
    auto tx = 0.01f * (float)global_translation[0];
    auto ty = 0.01f * (float)global_translation[1];
    auto tz = 0.01f * (float)global_translation[2];

    auto global_translation_converted = /*coordinate_transform **/ geom::Vector3{ tx,ty,tz };
    skeleton.inv_matrix_stack.push_back(geom::create_translation_matrix_44(global_translation_converted).inverse());

    bone.parent_index = parent_index;
    skeleton.bones.push_back(bone);

    for (int i = 0; i < node.GetChildCount(); ++i)
    {
        process_armature_node(*node.GetChild(i), skeleton, index, nodes_out);
    }
}

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
    FbxNode* mesh_node = nullptr;
    const FbxMesh* mesh = nullptr;
    for (int i = 0; i < child_count; ++i)
    {
        auto* child = root_node->GetChild(i);
        mesh = child->GetMesh();
        if (mesh != nullptr)
        {
            mesh_node = child;
            break;
        }
    }
    if (mesh == nullptr)
    {
        return {};
    }

    IndexedVertices<SkinnedVertex> vertices;

    //get skin info
    const FbxSkin* skin = nullptr;
    if (mesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin) > 0)
    {
        skin = static_cast<const FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
    }

    //get vertices
    FbxVector4* control_points = mesh->GetControlPoints();
    int control_points_count = mesh->GetControlPointsCount();
    auto& mesh_transform = mesh_node->EvaluateGlobalTransform();
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

        auto point = mesh_transform.MultT(control_points[control_point_index]);
        vertices.vertices.push_back({ geom::Vector3{
            0.01f * (float)point.mData[0],
            0.01f * (float)point.mData[1],
            0.01f * (float)point.mData[2]},
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

    //get skeleton
    FbxNode* armature = nullptr;
    for (int i = 0; i < root_node->GetChildCount(); ++i)
    {
        auto* child = root_node->GetChild(i);
        if (strcmp("Armature", child->GetName()) == 0)
        {
            armature = child;
        }
    }
    _ASSERT(armature);

    vertices.skeleton = std::make_unique<anim::Skeleton>();
    std::vector<FbxNode*> armature_nodes;
    process_armature_node(*armature, *vertices.skeleton, -1, armature_nodes);

    //loop over anim stacks
    for (int i = 0; i < scene->GetSrcObjectCount<FbxAnimStack>(); ++i)
    {
        FbxAnimStack* anim_stack = scene->GetSrcObject<FbxAnimStack>(i);
        float duration = 0.001f * (float)anim_stack->GetLocalTimeSpan().GetDuration().GetMilliSeconds();
        _ASSERT(anim_stack->GetSrcObjectCount<FbxAnimLayer>() == 1);
        FbxAnimLayer* anim_layer = anim_stack->GetSrcObject<FbxAnimLayer>(0);

        vertices.animations.push_back({ anim_stack->GetName(), anim::Animation(*vertices.skeleton) });
        anim::Animation& animation = vertices.animations.back().animation;

        //loop over frames and create a keyframe for each one
        constexpr float frame_dt = 1.f / 30.f;
        for (int frame = 0;; ++frame)
        {
            float time = frame_dt * frame;
            bool final_frame = false;
            if (time > duration)
            {
                time = duration;
                final_frame = true;
            }
            FbxTime ftime;
            ftime.SetMilliSeconds(time * 1000.f);

            //create a keyframe pose by looping over all the nodes
            anim::Pose pose;
            pose.skeleton = vertices.skeleton.get();

            pose.local_transforms.resize(armature_nodes.size());
            for (int transform_index = 0; transform_index < armature_nodes.size(); ++transform_index)
            {
                FbxNode* node = armature_nodes[transform_index];
                FbxAnimCurve* curve_trans_x = node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X);
                FbxAnimCurve* curve_trans_y = node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y);
                FbxAnimCurve* curve_trans_z = node->LclTranslation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z);
                FbxAnimCurve* curve_rot_x = node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X);
                FbxAnimCurve* curve_rot_y = node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y);
                FbxAnimCurve* curve_rot_z = node->LclRotation.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z);

                anim::Transform& transform = pose.local_transforms[transform_index];

                transform.translation.x = 0.01f * (float)node->LclTranslation.Get()[0];
                transform.translation.y = 0.01f * (float)node->LclTranslation.Get()[1];
                transform.translation.z = 0.01f * (float)node->LclTranslation.Get()[2];
                float deg_to_rad = 3.14159f / 180.f;
                float xrot = deg_to_rad * (float)node->LclRotation.Get()[0];
                float yrot = deg_to_rad * (float)node->LclRotation.Get()[1];
                float zrot = deg_to_rad * (float)node->LclRotation.Get()[2];
                ////translation
                //transform.translation.x = curve_trans_x ? curve_trans_x->Evaluate(ftime) : (float)node->LclTranslation.Get()[0];
                //transform.translation.y = curve_trans_y ? curve_trans_y->Evaluate(ftime) : (float)node->LclTranslation.Get()[1];
                //transform.translation.z = curve_trans_z ? curve_trans_z->Evaluate(ftime) : (float)node->LclTranslation.Get()[2];
                //
                ////rotation
                //float deg_to_rad = 3.14159f / 180.f;
                //float xrot = deg_to_rad * (curve_rot_x ? curve_rot_x->Evaluate(ftime) : (float)node->LclRotation.Get()[0]);
                //float yrot = deg_to_rad * (curve_rot_y ? curve_rot_y->Evaluate(ftime) : (float)node->LclRotation.Get()[1]);
                //float zrot = deg_to_rad * (curve_rot_z ? curve_rot_z->Evaluate(ftime) : (float)node->LclRotation.Get()[2]);
                //
                FbxEuler::EOrder rot_order;
                node->GetRotationOrder(FbxNode::EPivotSet::eSourcePivot, rot_order);
                
                geom::Matrix44 xrot_mat = geom::create_x_rotation_matrix_44(xrot);
                geom::Matrix44 yrot_mat = geom::create_x_rotation_matrix_44(yrot);
                geom::Matrix44 zrot_mat = geom::create_x_rotation_matrix_44(zrot);
                geom::Matrix44 rot_mat;
                
                switch (rot_order)
                {
                case FbxEuler::EOrder::eOrderXYZ:
                    rot_mat = zrot_mat * yrot_mat * xrot_mat;
                    break;
                case FbxEuler::EOrder::eOrderXZY:
                    rot_mat = yrot_mat * zrot_mat * xrot_mat;
                    break;
                case FbxEuler::EOrder::eOrderYZX:
                    rot_mat = xrot_mat * zrot_mat * yrot_mat;
                    break;
                case FbxEuler::EOrder::eOrderYXZ:
                    rot_mat = zrot_mat * xrot_mat * yrot_mat;
                    break;
                case FbxEuler::EOrder::eOrderZXY:
                    rot_mat = yrot_mat * xrot_mat * zrot_mat;
                    break;
                case FbxEuler::EOrder::eOrderZYX:
                    rot_mat = xrot_mat * yrot_mat * zrot_mat;
                    break;
                default:
                    _ASSERT(false); //wat
                    rot_mat = geom::Matrix44::identity();
                }
                transform.rotation = geom::create_quaternion_from_rotation_matrix(rot_mat);
            }

            animation.add_keyframe(std::move(pose), time);
            if (final_frame)
            {
                break;
            }
        }
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
    const anim::Animation* waterfowl_dance = nullptr;
#if true
    FBXManagerWrapper fbx_manager;
    FBXSceneWrapper test = fbx_manager.load("../../assets/fbx/cubeman_v2.fbx");
    auto vertices = get_vertices_from_fbx(test);
    VertexArray<SkinnedVertex> vao(
        VertexBuffer(std::move(vertices.vertices)),
        vertices.indices.data(),
        (int)vertices.indices.size());

    for (auto& anim : vertices.animations)
    {
        if (anim.name == "Armature|WaterfowlDance")
        {
            waterfowl_dance = &anim.animation;
        }
    }
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
        "layout(location = 1) in int bone;"

        "uniform mat4 camera;"
        "uniform mat4 bones[50];"
        "uniform mat4 inv_bones[50];"

        "void main()"
        "{"
        "mat4 bone_matrix = bones[bone];"
        "mat4 inv_bone_matrix = inv_bones[bone];"
        "gl_Position = camera * bone_matrix * inv_bone_matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
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

        auto test_mat = rotation_transform * rotation_transform.inverse();
        auto t2 = geom::Matrix<2, 2>{ {0.5f, 0.f, 0.f, 0.5f} };
        auto test_mat2 = t2 * t2.inverse();

        if (g_w_press) g_camera.translation += rotation_transform * geom::Vector3::unit_z() * g_timestep;
        if (g_s_press) g_camera.translation -= rotation_transform * geom::Vector3::unit_z() * g_timestep;
        if (g_a_press) g_camera.translation -= rotation_transform * geom::Vector3::unit_x() * g_timestep;
        if (g_d_press) g_camera.translation += rotation_transform * geom::Vector3::unit_x() * g_timestep;
        if (g_space_press) g_camera.translation += rotation_transform * geom::Vector3::unit_y() * g_timestep;
        if (g_control_press) g_camera.translation -= rotation_transform * geom::Vector3::unit_y() * g_timestep;


        //openGL housekeeping
        //glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //evaluate animation
        static float s_time = 0.f;
        s_time += g_timestep;
        std::vector<geom::Matrix44> mat_stack;
        if (waterfowl_dance)
        {
            mat_stack = waterfowl_dance->get_pose(s_time, true).get_matrix_stack();
        }

        //draw
        std::vector<geom::Matrix44> mat_stack_i(20, geom::Matrix44::identity());
        draw(vao, shader, mat_stack_i, mat_stack_i);
        //draw(vao, shader, mat_stack, vertices.skeleton->inv_matrix_stack);
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
        g_timestep = (float)std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() * 0.001f;
    }

    //close
    glfwTerminate();
    return 0;
}