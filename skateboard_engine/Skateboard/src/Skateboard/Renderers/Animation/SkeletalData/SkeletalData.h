#pragma once
#include "Skateboard/Core.h"
#include "Skateboard/Mathematics.h"

#include "Skateboard/Assets/Model/Model.h"
#include "Skateboard/Renderers/Animation/Animator/Animator.h"
#include "Skateboard/Renderers/Animation/Animation.h"

#include <vector>

namespace Skateboard
{

	struct Joint
	{
		Joint() = default;
		Joint(const Joint& rhs)
			:
			  Name(rhs.Name)
			, InverseBindPose(rhs.InverseBindPose)
			, ParentId(rhs.ParentId)
		{}

		Joint(Joint&& rhs) noexcept
			:
				Name(std::move(rhs.Name))
			,	InverseBindPose(std::move(rhs.InverseBindPose))
			,	ParentId(rhs.ParentId)
		{}

		std::string_view Name;
		Transform InverseBindPose;
		int32_t ParentId;// -1 if root.
	};



	class SKTBD_API Skeleton
	{
	public:
		Skeleton() = default;
		Skeleton(const Skeleton& rhs);
		Skeleton(Skeleton&& rhs) noexcept;
		~Skeleton() = default;

		void SetName(const std::string& name) { m_name = name; }

		int32_t AddJoint(const Joint& joint);
		int32_t GetJointById(const std::string_view& name) const;
		int32_t GetJointCount() const { return static_cast<int32_t>(m_Joints.size()); }

		std::vector<Joint>& GetJoints() { return m_Joints; }
		const std::vector<Joint>& GetJoints() const { return m_Joints; }

		auto Begin() { return m_Joints.begin(); }
		auto End() { return m_Joints.end(); }

	private:
		int32_t m_LocomotiveJointIndex; 
		int32_t m_JointCount; 
		std::string m_name;
		std::vector<Joint> m_Joints;
	};
	
	class SKTBD_API SkeletonPose
	{
	public:
		SkeletonPose();

		virtual void CreateBindPose(const Skeleton* const skeleton);

		virtual void CalculateGlobalPose(const glm::mat4x4* const transform = nullptr);
		virtual void CalculateLocalPose(const std::vector<Transform>& localPose);
		virtual void SetPoseFromAnimation(Animation& animation, const SkeletonPose& bindPose, const float time, const bool updateGlobalPose = true);
		virtual void Linear2Pose(const SkeletonPose& startPose, const SkeletonPose& endPose, const float alpha);

		const std::vector<Transform>& GetLocalPoses() const { return m_LocalPoses; }
		const std::vector<glm::mat4x4>& GetGlobalPoses() const { return m_GlobalPoses; }

		const Skeleton* GetSkeleton() const { return m_Skeleton; }

	private:
		std::vector<Transform> m_LocalPoses;
		std::vector<glm::mat4x4> m_GlobalPoses;
		const Skeleton* m_Skeleton;
	};

	class SKTBD_API SkinnedMesh : public Primitive
	{
	public:
		SkinnedMesh()=default;
		SkinnedMesh(const Skeleton& skeleton);
		virtual ~SkinnedMesh() {}

		void SetSkeleton(const Skeleton& skeleton);

		std::vector<glm::mat4x4>& BoneMatrices() { return m_BoneMatrices; }
		const std::vector<glm::mat4x4>& BoneMatrices() const { return m_BoneMatrices; }

		const SkeletonPose* GetPose() const { return &m_Pose; }

	protected:
		std::vector<glm::mat4x4> m_BoneMatrices;
		SkeletonPose m_Pose;
	};

}