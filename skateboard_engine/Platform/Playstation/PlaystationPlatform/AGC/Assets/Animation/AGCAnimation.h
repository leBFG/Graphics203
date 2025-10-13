#pragma once

#include "AGC/Memory/AGCMemoryAllocator.h"
#include <edgeanim.h>
#include "Skateboard/Renderers/Animation/Animation.h"


namespace Skateboard
{

	class AGCAnimation : public Animation
	{
	public:
		AGCAnimation();
		//AGCAnimation(MemoryHandle<EdgeAnimAnimation,sizeof(int16_t)>& handle);

		//void* GetAnimationData() final override { return &m_AnimationHandle.data; }

	private:
		//MemoryHandle<EdgeAnimAnimation, sizeof(int16_t)> m_AnimationHandle;
	};

}
