#include "sktbdpch.h"
#include "SkeletalData.h"

namespace Skateboard
{

//////////////////////////////////////// SKELETON /////////////////////////////////



	Skeleton::Skeleton(const Skeleton& rhs)
		:
			m_name(rhs.m_name)
		,	m_Joints(rhs.m_Joints)
	{}

	Skeleton::Skeleton(Skeleton&& rhs) noexcept
		:
			m_name(std::move(rhs.m_name))
		,	m_Joints(std::move(rhs.m_Joints))
	{}

	int32_t Skeleton::AddJoint(const Joint& joint)
	{
		m_Joints.push_back(joint);
		return static_cast<int32_t>(m_Joints.size()) - 1;
	}

	int32_t Skeleton::GetJointById(const std::string_view& name) const
	{
		int32_t result = -1;

		for (int32_t i = 0; i < static_cast<int32_t>(m_Joints.size()); ++i)
		{
			if (m_Joints[i].Name == name)
			{
				result = i;
				break;
			}
		}

		return result;
	}


//////////////////////////////////////// SKELETON POSE /////////////////////////////////

	SkeletonPose::SkeletonPose()
	{
	}

	void SkeletonPose::CalculateGlobalPose(const glm::mat4x4* const transform)
	{
		if (transform != nullptr)
		{
			const std::vector<Joint>& joints = m_Skeleton->GetJoints();

			for (uint32_t jointNum = 0; jointNum < joints.size(); jointNum++)
			{
				const Joint& joint = joints[jointNum];
				Transform localPoseMatrix = m_LocalPoses[jointNum];
				glm::mat4x4 globalPoseMatrix;
				if (joint.ParentId == -1)
				{
					globalPoseMatrix = localPoseMatrix.AsMatrix();
					if (transform) {
						globalPoseMatrix = globalPoseMatrix * (*transform);
					}
				}
				else {
					globalPoseMatrix = localPoseMatrix.AsMatrix() * m_GlobalPoses[joint.ParentId];
				}

				if (m_GlobalPoses.size() > jointNum) {
					m_GlobalPoses[jointNum] = globalPoseMatrix;
				}
				else {
					m_GlobalPoses.push_back(globalPoseMatrix);
				}
			}
		}
	}

	void SkeletonPose::CalculateLocalPose(const std::vector<Transform>& localPose)
	{
		if (m_Skeleton != nullptr)
		{
			const std::vector<Joint>& joints = m_Skeleton->GetJoints();

			for (uint32_t jointNum = 0; jointNum < joints.size(); jointNum++)
			{
				const Joint& joint = joints[jointNum];
				glm::mat4x4 localPoseMatrix;
				const Transform& globalPoseMatrix = localPose[jointNum];
				if (joint.ParentId == -1) {
					localPoseMatrix = globalPoseMatrix.AsMatrix();
				}
				else{
					glm::mat4x4 invParentMatrix = glm::inverse(localPose[joint.ParentId].AsMatrix());
					localPoseMatrix = globalPoseMatrix.AsMatrix() * invParentMatrix;
				}

				m_LocalPoses[jointNum] = localPoseMatrix;
			}
		}
	}

	void SkeletonPose::SetPoseFromAnimation(Animation& animation, const SkeletonPose& bindPose, const float time, const bool updateGlobalPose)
	{

		int32_t jointIndex = 0;

		const std::vector<Joint>& joints = m_Skeleton->GetJoints();

		for (auto itr = m_LocalPoses.begin(); itr != m_LocalPoses.end(); ++itr, ++jointIndex)
		{
			const AnimationNode* animNode = animation.FindNode(joints[jointIndex].ParentId);
			Transform& jointPose = m_LocalPoses[jointIndex];

			if (jointIndex != 1) {
				animNode = nullptr;
			}

			if (animNode)
			{
				if (animNode->GetType() == AnimationNode::NodeType_::NodeType_Transform) // this should always be true since the find uses the joint transform name
				{
					auto transformNode = static_cast<const TransformNode*>(animNode);

					// scale
					if (transformNode->GetScaleKeys().size() > 0.f) {
						auto scale = transformNode->GetVector(time);

						// set scale
						jointPose.Scale = scale;
					}
					else {
						jointPose.Scale = m_LocalPoses[jointIndex].Scale;
					}


					// rotation
					if (transformNode->GetRotationKeys().size() > 0.f) {
						auto rotation = transformNode->GetRotation(time);
						// set rotation
						jointPose.Rotation = rotation;
					}
					else {
						jointPose.Rotation = m_LocalPoses[jointIndex].Rotation;
					}

					// translation
					if (transformNode->GetTranslationKeys().size() > 0.f) {
						auto translation = transformNode->GetVector(time);
						// set translation
						jointPose.Translation = translation;
					}
					else {
						jointPose.Translation = m_LocalPoses[jointIndex].Translation;
					}
				}
			}
			else
			{
				jointPose = bindPose.GetLocalPoses()[jointIndex];
			}
		}

		if (updateGlobalPose) {
			CalculateGlobalPose();
		}
	}

	void SkeletonPose::Linear2Pose(const SkeletonPose& startPose, const SkeletonPose& endPose, const float alpha)
	{
	}

	void SkeletonPose::CreateBindPose(const Skeleton* const skeleton)
	{
		if (skeleton != nullptr)
		{
			m_LocalPoses.clear();
			m_GlobalPoses.clear();

			const std::vector<Joint>& joints = skeleton->GetJoints();

			for (uint32_t jointNum = 0; jointNum < static_cast<uint32_t>(skeleton->GetJoints().size()); jointNum++)
			{
				glm::mat4x4 localMatrix;
				glm::mat4x4 globalBindMatrix;

				const Joint& joint = joints[jointNum];
				if (joint.ParentId == -1)
				{
					localMatrix = glm::inverse(joint.InverseBindPose.AsMatrix());
					globalBindMatrix = localMatrix;
				}
				else
				{
					const Joint& parent_joint = joints[joint.ParentId];
					glm::mat4x4 invParentMatrix = parent_joint.InverseBindPose.AsMatrix();

					glm::inverse(joint.InverseBindPose.AsMatrix());
					localMatrix = globalBindMatrix * invParentMatrix;
				}

				Transform jointPose;
				jointPose = localMatrix;

				m_LocalPoses.push_back(jointPose);
				m_GlobalPoses.push_back(globalBindMatrix);
			}

			m_Skeleton = skeleton;
		}
	}

	SkinnedMesh::SkinnedMesh(const Skeleton& skeleton)
	{
		m_Pose.CreateBindPose(&skeleton);
		m_BoneMatrices.resize(skeleton.GetJointCount());
	}

	void SkinnedMesh::SetSkeleton(const Skeleton& skeleton)
	{
		m_BoneMatrices.clear();
		m_Pose.CreateBindPose(&skeleton);
		m_BoneMatrices.resize(skeleton.GetJointCount());
	}

}
