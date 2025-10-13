#pragma once

#include <edgeanim.h>
#include "Skateboard/Renderers/Animation/SkeletalData/SkeletalData.h"

namespace Skateboard
{
	struct AGCEdgeSkeletonData
	{
		EdgeAnimSkeleton* m_SkeletonPose;
		EdgeAnimJointTransform m_SkeletonRootJointTrans;
		EdgeAnimContext m_AnimationContext;
	};

	class AGCSkeletonPose : public SkeletonPose
	{
	public:

		//virtual void* GetSkeletonData() override { return reinterpret_cast<void*>(&m_SkeletonData); }


	private:
		AGCEdgeSkeletonData m_SkeletonData;
	};

	

}
