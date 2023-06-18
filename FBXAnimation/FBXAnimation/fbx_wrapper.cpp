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
        const FbxSkin* skin = nullptr;
        if (mesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin) > 0)
        {
            skin = static_cast<const FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
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
            content.vertices.push_back({ geom::Vector3{
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
        process_armature_node(*armature, *content.skeleton, -1, armature_nodes);

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
