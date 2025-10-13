#pragma once
#include "Skateboard/Renderers/Animation/Animator/Animator.h"

namespace Skateboard
{

	class AGCAnimator : public Animator
	{
	public:
		AGCAnimator();
		virtual ~AGCAnimator() override;

		virtual void UpdateAnimationImpl(SkeletonPose* skeleton, Animation* animation, float elapsedTime) final override;

		virtual void LinearBlendImpl(SkeletonPose* poseA, SkeletonPose* poseB, SkeletonPose& out, float alpha) final override;

	private:
	};

}