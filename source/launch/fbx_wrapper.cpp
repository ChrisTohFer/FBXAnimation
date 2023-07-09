#include "fbx_wrapper.h"

#include <iostream>

//housekeeping
FBXManagerWrapper::FBXManagerWrapper()
{
    m_manager = FbxManager::Create();
    FbxIOSettings* ioSettings = FbxIOSettings::Create(m_manager, "");
    m_manager->SetIOSettings(ioSettings);
}

FBXManagerWrapper::~FBXManagerWrapper()
{
    if (m_manager)
    {
        m_manager->Destroy();
        m_manager = nullptr;
    }
}

//loading

namespace
{
    //scene loading and unloading

    FbxScene* load(FbxManager& manager, const char* filename)
    {
        FbxImporter* importer = FbxImporter::Create(&manager, "");

        if (!importer->Initialize(filename, -1, manager.GetIOSettings()))
        {
            //error
            printf("Call to FbxImporter::Initialize() failed.\n");
            printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
            return nullptr;
        }

        FbxScene* scene = FbxScene::Create(&manager, filename);
        importer->Import(scene);
        importer->Destroy();

        //only want to draw triangles and not quads, so convert the scene immediately
        FbxGeometryConverter converter(&manager);
        converter.Triangulate(scene, true);

        return scene;
    }

    void unload(FbxScene& scene)
    {
        scene.Destroy();
    }

    //context

    struct LoadContext
    {
        FbxScene& scene;
        FbxFileContent& result;

        FbxNode* root_node = nullptr;
        FbxNode* mesh_node = nullptr;
        FbxMesh* mesh = nullptr;
        FbxSkin* skin = nullptr;

        std::vector<FbxNode*> skeleton_nodes;
    };

    //conversions

    geom::Quaternion right_to_left_hand(geom::Quaternion q)
    {
        return { -q.x, -q.y, q.z, q.w };
    }

    geom::Vector3 right_to_left_hand(geom::Vector3 v)
    {
        return { v.x, v.y, -v.z };
    }

    geom::Quaternion get_quaternion_from_fbx_euler(float x, float y, float z, FbxEuler::EOrder order)
    {

        geom::Matrix44 xrot_mat = geom::create_x_rotation_matrix_44(x);
        geom::Matrix44 yrot_mat = geom::create_y_rotation_matrix_44(y);
        geom::Matrix44 zrot_mat = geom::create_z_rotation_matrix_44(z);
        geom::Matrix44 rot_mat;

        switch (order)
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
        return geom::Quaternion::from_rotation_matrix(rot_mat);
    }

    //skin + skeleton processing

    int get_skin_index(const FbxSkin* skin, int control_point_index)
    {
        int skinned_index = 0;
        if (skin == nullptr)
        {
            return skinned_index;
        }

        //iterate over clusters and take the index of the cluster with the highest weighting on this control point
        int cluster_count = skin->GetClusterCount();
        double skinned_weight = 0.f;
        for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
        {
            const FbxCluster* cluster = skin->GetCluster(cluster_index);
            int control_point_indices_count = cluster->GetControlPointIndicesCount();
            int* control_point_indices = cluster->GetControlPointIndices();
            double* control_point_weights = cluster->GetControlPointWeights();

            //iterate the control points in this cluster to see if it references the control point we are interested in
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

        return skinned_index;
    }

    void process_skeleton_nodes(LoadContext& context)
    {
        anim::Skeleton& skeleton = *context.result.skeleton;

        //resize the bone and inverse matrix stack arrays to the cluster count
        int cluster_count = context.skin->GetClusterCount();
        skeleton.bones.resize(cluster_count);
        skeleton.inv_matrix_stack.resize(cluster_count);

        //iterate over clusters and get their global transforms and hierarchy
        for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
        {
            FbxCluster* cluster = context.skin->GetCluster(cluster_index);
            FbxNode* linked_node = cluster->GetLink();
            context.skeleton_nodes.push_back(linked_node);

            //debug
            std::cout << linked_node->GetName() << "\n";
            //debug

            anim::Skeleton::Bone bone;
            auto& global_transform = linked_node->EvaluateGlobalTransform();
            auto global_translation = global_transform.GetT();
            auto global_rotation = global_transform.GetR();

            bone.global_transform.translation.x = (float)global_translation[0];
            bone.global_transform.translation.y = (float)global_translation[1];
            bone.global_transform.translation.z = (float)global_translation[2];
            bone.global_transform.translation = right_to_left_hand(bone.global_transform.translation) * 0.01f;

            float deg_to_rad = geom::PI / 180.f;
            float xrot = deg_to_rad * (float)global_rotation[0];
            float yrot = deg_to_rad * (float)global_rotation[1];
            float zrot = deg_to_rad * (float)global_rotation[2];
            bone.global_transform.rotation = right_to_left_hand(get_quaternion_from_fbx_euler(xrot, yrot, zrot, FbxEuler::EOrder::eOrderXYZ));

            //global_transform
            skeleton.inv_matrix_stack[cluster_index] = (
                geom::create_translation_matrix_44(bone.global_transform.translation) *
                geom::create_rotation_matrix_from_quaternion(bone.global_transform.rotation))
                .inverse();

            bone.parent_index = -1;
            for (int i = 0; i < context.skeleton_nodes.size(); ++i)
            {
                auto& node_out = context.skeleton_nodes[i];
                if (node_out == linked_node->GetParent())
                {
                    bone.parent_index = i;
                }
            }

            //we're assuming that clusters always come after their parents, assert to make sure this is true
            _ASSERT(cluster_index == 0 || bone.parent_index != -1);

            skeleton.bones[cluster_index] = bone;
        }
    }

    //anim + keyframe processing

    anim::Pose process_keyframe(
        FbxTime& time,
        const anim::Skeleton& skeleton,
        const std::vector<FbxNode*>& skeleton_nodes,
        FbxAnimLayer* anim_layer)
    {
        anim::Pose pose;
        pose.skeleton = &skeleton;

        //iterate over bones
        std::vector<FbxAMatrix> global_transforms;
        global_transforms.reserve(skeleton_nodes.size());
        pose.local_transforms.resize(skeleton_nodes.size());

        for (int transform_index = 0; transform_index < skeleton_nodes.size(); ++transform_index)
        {
            FbxNode* node = skeleton_nodes[transform_index];

            //cache the global transform
            global_transforms.push_back(node->EvaluateGlobalTransform(time));

            //we don't animate scale, set to zero
            global_transforms.back().SetS(FbxVector4{ 1.f, 1.f, 1.f, 1.f });

            //assert if there is any significant scaling as we don't account for this
            FbxAnimCurve* curve_scale_x = node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_X);
            FbxAnimCurve* curve_scale_y = node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Y);
            FbxAnimCurve* curve_scale_z = node->LclScaling.GetCurve(anim_layer, FBXSDK_CURVENODE_COMPONENT_Z);
            geom::Vector3 scale;
            scale.x = curve_scale_x ? curve_scale_x->Evaluate(time) : (float)node->LclScaling.Get()[0];
            scale.y = curve_scale_y ? curve_scale_y->Evaluate(time) : (float)node->LclScaling.Get()[1];
            scale.z = curve_scale_z ? curve_scale_z->Evaluate(time) : (float)node->LclScaling.Get()[2];
            _ASSERT(scale.x > 0.999f && scale.x < 1.001f);
            _ASSERT(scale.y > 0.999f && scale.y < 1.001f);
            _ASSERT(scale.z > 0.999f && scale.z < 1.001f);

            anim::Transform& transform = pose.local_transforms[transform_index];

            //get the local transform by multiplying global transform by inverse of parent transform
            int parent_index = skeleton.bones[transform_index].parent_index;
            FbxAMatrix local_transform = parent_index == -1 ?
                global_transforms[transform_index] :
                global_transforms[parent_index].Inverse() * global_transforms[transform_index];

            //extract the translation and rotation
            auto fbx_translation = local_transform.GetT();
            auto fbx_rotation = local_transform.GetR();

            transform.translation.x = (float)fbx_translation[0];
            transform.translation.y = (float)fbx_translation[1];
            transform.translation.z = (float)fbx_translation[2];

            float deg_to_rad = geom::PI / 180.f;
            float xrot = deg_to_rad * (float)fbx_rotation[0];
            float yrot = deg_to_rad * (float)fbx_rotation[1];
            float zrot = deg_to_rad * (float)fbx_rotation[2];
            transform.rotation = get_quaternion_from_fbx_euler(xrot, yrot, zrot, FbxEuler::EOrder::eOrderXYZ);

            //convert coordinate systems
            transform.translation = right_to_left_hand(transform.translation) * 0.01f;
            transform.rotation = right_to_left_hand(transform.rotation.normalized());
        }

        return pose;
    }

    void process_animations(LoadContext& context)
    {
        auto& animations = context.result.animations;
        auto& skeleton = *context.result.skeleton;
        auto& scene = context.scene;

        //loop over anim stacks
        for (int i = 0; i < scene.GetSrcObjectCount<FbxAnimStack>(); ++i)
        {
            FbxAnimStack* anim_stack = scene.GetSrcObject<FbxAnimStack>(i);
            std::cout << "Loading anim stack: " << anim_stack->GetName() << "\n";

            float duration = 0.001f * (float)anim_stack->GetLocalTimeSpan().GetDuration().GetMilliSeconds();
            _ASSERT(anim_stack->GetSrcObjectCount<FbxAnimLayer>() == 1);
            FbxAnimLayer* anim_layer = anim_stack->GetSrcObject<FbxAnimLayer>(0);

            animations.push_back({ anim_stack->GetName(), anim::Animation(skeleton) });
            anim::Animation& animation = animations.back().animation;

            scene.SetCurrentAnimationStack(anim_stack);

            //loop over frames and create a keyframe for each one
            constexpr float frame_dt = 1.f / 30.f;
            for (int frame = 0;; ++frame)
            {
                float time = frame_dt * frame;
                
                //if this is the final frame, trim the time and set a flag so we remember to exit
                bool final_frame = false;
                if (time > duration)
                {
                    time = duration;
                    final_frame = true;
                }

                //create the keyframe
                FbxTime ftime;
                ftime.SetMilliSeconds((FbxLongLong)(time * 1000.f));
                animation.add_keyframe(
                    process_keyframe(ftime, skeleton, context.skeleton_nodes, anim_layer),
                    time);

                if (final_frame)
                {
                    break;
                }
            }
        }
    }

    //main funcs

    void read_file_content(LoadContext& context)
    {
        //get root node
        context.root_node = context.scene.GetRootNode();
        if (context.root_node == nullptr)
        {
            //error
            return;
        }
        FbxNode& root_node = *context.root_node;

        //get mesh
        int root_child_count = root_node.GetChildCount();

        for (int i = 0; i < root_child_count; ++i)
        {
            auto* child = root_node.GetChild(i);
            context.mesh = child->GetMesh();
            if (context.mesh != nullptr)
            {
                context.mesh_node = child;
                break;
            }
        }
        if (context.mesh == nullptr)
        {
            //error
            return;
        }
        FbxMesh& mesh = *context.mesh;

        //get skin info
        if (mesh.GetDeformerCount(FbxDeformer::EDeformerType::eSkin) > 0)
        {
            context.skin = static_cast<FbxSkin*>(mesh.GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
        }

        //get vertices
        FbxVector4* mesh_control_points = mesh.GetControlPoints();
        int mesh_control_point_count = mesh.GetControlPointsCount();
        auto& mesh_transform = context.mesh_node->EvaluateGlobalTransform();

        std::vector<SkinnedVertex>& vertices = context.result.vertices;
        vertices.reserve(mesh_control_point_count);
        for (int control_point_index = 0; control_point_index < mesh_control_point_count; ++control_point_index)
        {
            int skinned_index = get_skin_index(context.skin, control_point_index);

            auto point = mesh_transform.MultT(mesh_control_points[control_point_index]);
            vertices.push_back({ 
                right_to_left_hand(geom::Vector3{
                    0.01f * (float)point.mData[0],
                    0.01f * (float)point.mData[1],
                    0.01f * (float)point.mData[2]}),
                skinned_index
                });

        }

        //get triangles
        int polygon_count = mesh.GetPolygonCount();
        std::vector<unsigned int>& indices = context.result.indices;
        indices.reserve(polygon_count * 3);
        for (int i = 0; i < polygon_count; ++i)
        {
            int num_polygon_vertices = mesh.GetPolygonSize(i);
            _ASSERT(num_polygon_vertices == 3);

            indices.push_back(mesh.GetPolygonVertex(i, 0));
            indices.push_back(mesh.GetPolygonVertex(i, 1));
            indices.push_back(mesh.GetPolygonVertex(i, 2));
        }

        context.result.skeleton = std::make_unique<anim::Skeleton>();
        process_skeleton_nodes(context);

        //get animations
        process_animations(context);
    }

}

FbxFileContent FBXManagerWrapper::load_file_content(const char* filename)
{
    FbxFileContent result;

    _ASSERT(m_manager);
    FbxScene* scene = load(*m_manager, filename);

    if (scene == nullptr)
    {
        //invalid file
        return result;
    }

    LoadContext context = { *scene, result };

    read_file_content(context);

    unload(*scene);

    return result;
}
