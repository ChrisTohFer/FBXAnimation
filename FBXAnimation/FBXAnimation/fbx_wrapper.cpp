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
        return geom::create_quaternion_from_rotation_matrix(rot_mat);
    }

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

    void process_armature_nodes(FbxSkin& skin, anim::Skeleton& skeleton, std::vector<FbxNode*>& nodes_out)
    {
        int cluster_count = skin.GetClusterCount();
        skeleton.bones.resize(cluster_count);
        skeleton.inv_matrix_stack.resize(cluster_count);
        for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
        {
            FbxCluster* cluster = skin.GetCluster(cluster_index);
            FbxNode* linked_node = cluster->GetLink();
            nodes_out.push_back(linked_node);

            //debug
            std::cout << linked_node->GetName() << "\n";
            //debug

            anim::Skeleton::Bone bone;
            auto& global_transform = linked_node->EvaluateGlobalTransform();
            auto global_translation = global_transform.GetT();
            auto global_rotation = global_transform.GetR();

            bone.global_transform.translation.x = 0.01f * (float)global_translation[0];
            bone.global_transform.translation.y = 0.01f * (float)global_translation[1];
            bone.global_transform.translation.z = 0.01f * (float)global_translation[2];
            bone.global_transform.translation = right_to_left_hand(bone.global_transform.translation);

            float deg_to_rad = 3.14159f / 180.f;
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
            for (int i = 0; i < nodes_out.size(); ++i)
            {
                auto& node_out = nodes_out[i];
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

    anim::Pose process_keyframe(
        FbxTime& time,
        const anim::Skeleton& skeleton,
        std::vector<FbxNode*> armature_nodes,
        FbxAnimLayer* anim_layer)
    {
        anim::Pose pose;
        pose.skeleton = &skeleton;

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
            
            //check if this is the root
            // -for the root we evaluate the global transform
            // -for all other bones we evaluate the local transform
            if (transform_index != 0)
            {
                //translation
                transform.translation.x = curve_trans_x ? curve_trans_x->Evaluate(time) : (float)node->LclTranslation.Get()[0];
                transform.translation.y = curve_trans_y ? curve_trans_y->Evaluate(time) : (float)node->LclTranslation.Get()[1];
                transform.translation.z = curve_trans_z ? curve_trans_z->Evaluate(time) : (float)node->LclTranslation.Get()[2];
                transform.translation = right_to_left_hand(transform.translation);

                //rotation
                float deg_to_rad = 3.14159f / 180.f;
                float xrot = deg_to_rad * (curve_rot_x ? curve_rot_x->Evaluate(time) : (float)node->LclRotation.Get()[0]);
                float yrot = deg_to_rad * (curve_rot_y ? curve_rot_y->Evaluate(time) : (float)node->LclRotation.Get()[1]);
                float zrot = deg_to_rad * (curve_rot_z ? curve_rot_z->Evaluate(time) : (float)node->LclRotation.Get()[2]);

                FbxEuler::EOrder rot_order;
                node->GetRotationOrder(FbxNode::EPivotSet::eSourcePivot, rot_order);

                transform.rotation = right_to_left_hand(get_quaternion_from_fbx_euler(xrot, yrot, zrot, rot_order));
            }
            else
            {
                //apply global transformation to the root node
                auto& global_transform = node->EvaluateGlobalTransform(time);
                auto fbx_global_translation = global_transform.GetT();
                auto fbx_global_rotation = global_transform.GetR();

                geom::Vector3 global_translation;
                global_translation.x = 0.01f * (float)fbx_global_translation[0];
                global_translation.y = 0.01f * (float)fbx_global_translation[1];
                global_translation.z = 0.01f * (float)fbx_global_translation[2];
                global_translation = right_to_left_hand(global_translation);

                geom::Quaternion global_rotation;
                float deg_to_rad = 3.14159f / 180.f;
                float xrot = deg_to_rad * (float)fbx_global_rotation[0];
                float yrot = deg_to_rad * (float)fbx_global_rotation[1];
                float zrot = deg_to_rad * (float)fbx_global_rotation[2];
                global_rotation = right_to_left_hand(get_quaternion_from_fbx_euler(xrot, yrot, zrot, FbxEuler::EOrder::eOrderXYZ));

                //global_transform
                transform.translation = global_translation;
                transform.rotation = global_rotation;
            }
        }

        return pose;
    }

    void process_animations(FbxScene& scene, std::vector<FbxNode*>& armature_nodes, FbxFileContent& content)
    {
        //loop over anim stacks
        for (int i = 0; i < scene.GetSrcObjectCount<FbxAnimStack>(); ++i)
        {
            FbxAnimStack* anim_stack = scene.GetSrcObject<FbxAnimStack>(i);
            float duration = 0.001f * (float)anim_stack->GetLocalTimeSpan().GetDuration().GetMilliSeconds();
            _ASSERT(anim_stack->GetSrcObjectCount<FbxAnimLayer>() == 1);
            FbxAnimLayer* anim_layer = anim_stack->GetSrcObject<FbxAnimLayer>(0);

            content.animations.push_back({ anim_stack->GetName(), anim::Animation(*content.skeleton) });
            anim::Animation& animation = content.animations.back().animation;

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
                    process_keyframe(ftime, *content.skeleton, armature_nodes, anim_layer),
                    time);

                if (final_frame)
                {
                    break;
                }
            }
        }
    }

    void read_file_content(FbxScene& scene, FbxFileContent& content)
    {
        //get root node
        FbxNode* root_node = scene.GetRootNode();
        if (root_node == nullptr)
        {
            //error
            return;
        }

        //get mesh
        int root_child_count = root_node->GetChildCount();

        FbxNode* mesh_node = nullptr;   //non-const so we can evaluate the transform
        const FbxMesh* mesh = nullptr;
        for (int i = 0; i < root_child_count; ++i)
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
            //error
            return;
        }

        //get skin info
        FbxSkin* skin = nullptr;
        if (mesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin) > 0)
        {
            skin = static_cast<FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
        }

        //get vertices
        FbxVector4* mesh_control_points = mesh->GetControlPoints();
        int mesh_control_point_count = mesh->GetControlPointsCount();
        auto& mesh_transform = mesh_node->EvaluateGlobalTransform();

        content.vertices.reserve(mesh_control_point_count);
        for (int control_point_index = 0; control_point_index < mesh_control_point_count; ++control_point_index)
        {
            int skinned_index = get_skin_index(skin, control_point_index);

            auto point = mesh_transform.MultT(mesh_control_points[control_point_index]);
            content.vertices.push_back({ 
                right_to_left_hand(geom::Vector3{
                    0.01f * (float)point.mData[0],
                    0.01f * (float)point.mData[1],
                    0.01f * (float)point.mData[2]}),
                skinned_index
                });

        }

        //get triangles
        int polygon_count = mesh->GetPolygonCount();
        for (int i = 0; i < polygon_count; ++i)
        {
            int num_polygon_vertices = mesh->GetPolygonSize(i);
            _ASSERT(num_polygon_vertices == 3);

            content.indices.push_back(mesh->GetPolygonVertex(i, 0));
            content.indices.push_back(mesh->GetPolygonVertex(i, 1));
            content.indices.push_back(mesh->GetPolygonVertex(i, 2));
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

        content.skeleton = std::make_unique<anim::Skeleton>();
        std::vector<FbxNode*> armature_nodes;
        process_armature_nodes(*skin, *content.skeleton, armature_nodes);

        //get animations
        process_animations(scene, armature_nodes, content);
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

    read_file_content(*scene, result);

    unload(*scene);

    return result;
}
