#include "sktbdpch.h"
#include "Animation.h"

#include "Skateboard/Renderers/Renderer.h"

#define SKTBD_LOG_COMPONENT "ANIMATION"
#include "Skateboard/Log.h"

namespace Skateboard
{
	AnimationNode* AnimationNode::Create()
	{
		return nullptr;
	}

	AnimationNode::AnimationNode(NodeType_ type)
		:
		m_Type(type)
	{}

	AnimationNode::~AnimationNode()
	{}

	TransformNode::TransformNode()
		:
		AnimationNode(NodeType_Transform)
	{}

	TransformNode::~TransformNode()
	{}

	float TransformNode::GetKeyMaxTime()
	{
		float maxTime = 0.f;
		float keyTime = 0.f;

		if (!GetScaleKeys().empty()) {
			keyTime = GetScaleKeys().back().Time;
			if (keyTime > maxTime) {
				maxTime = keyTime;
			}
		}

		if (!GetRotationKeys().empty()) {
			keyTime = GetRotationKeys().back().Time;
			if (keyTime > maxTime) {
				maxTime = keyTime;
			}
		}
		
		if (!GetTranslationKeys().empty()) {
			keyTime = GetTranslationKeys().back().Time;
			if (keyTime > maxTime) {
				maxTime = keyTime;
			}
		}

		return maxTime;
	}

	glm::quat TransformNode::GetRotation(float time) const
	{
		glm::quat result(1,0,0,0);

		const QuaternionKey* pPrevKey = nullptr;
		const QuaternionKey* pNextKey = nullptr;

		uint32_t keyIndex;
		for (keyIndex = 0; keyIndex < static_cast<uint32_t>(m_Rotation.size()); keyIndex++)
		{

			if (m_Rotation[keyIndex].Time > time)
			{
				pNextKey = &m_Rotation[keyIndex];

				if (keyIndex > 0)
					pPrevKey = &m_Rotation[keyIndex - 1];

				break;
			}
		}

		if (keyIndex == m_Rotation.size())
		{
			pNextKey = &m_Rotation[keyIndex - 1];
		}

		if (pPrevKey)
		{
			if (pNextKey){
				float t = (time - pPrevKey->Time) / (pNextKey->Time - pPrevKey->Time);
				result = glm::slerp(pPrevKey->Quaternion, pNextKey->Quaternion, t);
			}
			else {
				result = pPrevKey->Quaternion;
			}
;
		}
		else if (pNextKey) {
			result = pNextKey->Quaternion;

		}

		return result;
	}

	glm::vec4 TransformNode::GetVector(float time) const
	{
		glm::vec4 result(0.f, 0.f, 0.f, 0.f);

		const VectorKey* pPrevKey = nullptr;
		const VectorKey* pNextKey = nullptr;

		uint32_t keyIndex;

		for (keyIndex = 0; keyIndex < static_cast<uint32_t>(m_Translation.size()); keyIndex++)
		{

			if (m_Translation[keyIndex].Time > time)
			{
				pNextKey = &m_Translation[keyIndex];

				if (keyIndex > 0)
					pPrevKey = &m_Translation[keyIndex - 1];

				break;
			}
		}

		if (keyIndex == m_Translation.size())
		{
			pNextKey = &m_Translation[keyIndex - 1];
		}

		if (pPrevKey)
		{
			if (pNextKey)
			{
				float t = (time - pPrevKey->Time) / (pNextKey->Time - pPrevKey->Time);
				result = glm::lerp(pPrevKey->Vector, pNextKey->Vector, t);
			}
			else
				result = pPrevKey->Vector;
		}
		else if (pNextKey)
			result = pNextKey->Vector;

		return result;
	}

	ChannelNode::ChannelNode()
		:
		AnimationNode(NodeType_Channel)
	{}

	ChannelNode::~ChannelNode()
	{}

	float ChannelNode::GetKeyMaxTime()
	{
		return 0.0f;
	}

	const AnimationNode* Animation::FindNode(int32_t id)
	{
		const AnimationNode* result = nullptr;
		std::map<int32_t, AnimationNode*>::const_iterator itr = m_AnimationNodes.find(id);
		if (itr != m_AnimationNodes.end()) {
			result = itr->second;
		}

		return result;
	}

	Animation::Animation()
	{
	}

	Animation::~Animation()
	{
	}

}
