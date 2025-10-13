#include "sktbdpch.h"
#include "AGCAnimator.h"

#include "AGC/Assets/Animation/AGCAnimation.h"
#include "AGC/Assets/Animation/AGCSkeletalData.h"

namespace Skateboard
{
	AGCAnimator::AGCAnimator()
	{

	}

	AGCAnimator::~AGCAnimator()
	{
	}
	
	void AGCAnimator::UpdateAnimationImpl(SkeletonPose* skeleton, Animation* animation, float elapsedTime)
	{

		// push animations onto stack
		// push empty pose onto the stack
		AGCEdgeSkeletonData* edgeAnimData = 0;// static_cast<AGCEdgeSkeletonData*>(skeleton->GetSkeletonData());

		edgeAnimPoseStackPush(&edgeAnimData->m_AnimationContext);

		// get pose at the top of the stack
		EdgeAnimPoseInfo pose;
		edgeAnimPoseStackGetPose(&edgeAnimData->m_AnimationContext, &pose);

		EdgeAnimAnimation* edgeAnim = static_cast<EdgeAnimAnimation*>(animation->GetAnimationData());
		
		// evaluate animation to pose
		float evalTime = fmodf(elapsedTime, edgeAnim->duration);
		edgeAnimEvaluate(edgeAnim, edgeAnimData->m_SkeletonPose, &pose, evalTime);


		glm::mat4x4 mat;
		auto pvalue = glm::value_ptr(mat);

		// convert pose to world matrices
		edgeAnimLocalJointsToWorldMatrices4x4(pvalue, pose.jointArray, 
			&edgeAnimData->m_SkeletonRootJointTrans, 
			edgeAnimData->m_SkeletonPose->jointLinkage, 
			edgeAnimData->m_SkeletonPose->numJointLinkages);

		// pop the stack
		edgeAnimPoseStackPop(&edgeAnimData->m_AnimationContext);
	}

	void AGCAnimator::LinearBlendImpl(SkeletonPose* poseA, SkeletonPose* poseB, SkeletonPose& out, float alpha)
	{
	}

}
