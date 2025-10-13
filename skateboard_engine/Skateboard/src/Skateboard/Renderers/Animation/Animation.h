#pragma once
#include "Skateboard/Core.h"
#include "Skateboard/Mathematics.h"

#include <vector>
#include <map>

namespace Skateboard
{
	struct VectorKey
	{
		glm::vec4 Vector;
		float Time;
	};

	struct QuaternionKey
	{
		glm::quat Quaternion;
		float Time;
	};

	struct ChannelKey
	{
		float Value;
		float Time;
	};

	class SKTBD_API AnimationNode
	{
	public:
		enum NodeType_
		{
			NodeType_Transform = 0,
			NodeType_Channel
		};
	public:

		static AnimationNode* Create();

		AnimationNode(NodeType_ type);
		virtual ~AnimationNode();

		virtual float GetKeyMaxTime() = 0;

		NodeType_ GetType() const { return m_Type; }

	protected:
		NodeType_ m_Type;

	private:
		int32_t m_Id;
	};

	class SKTBD_API TransformNode : public AnimationNode
	{
	public:
		TransformNode();
		~TransformNode() override;

		glm::quat GetRotation(float time) const;
		glm::vec4 GetVector(float time) const;

		const std::vector<VectorKey>& GetTranslationKeys() const { return m_Translation; }
		std::vector<VectorKey>& GetTranslationKeys()			 { return m_Translation; }
		const std::vector<QuaternionKey>& GetRotationKeys() const	 { return m_Rotation; }
		std::vector<QuaternionKey>& GetRotationKeys()				 { return m_Rotation; }
		const std::vector<VectorKey>& GetScaleKeys() const		 { return m_Scale; }
		std::vector<VectorKey>& GetScaleKeys()					 { return m_Scale; }

		float GetKeyMaxTime() override;

	private:
		std::vector<VectorKey> m_Translation;
		std::vector<QuaternionKey> m_Rotation;
		std::vector<VectorKey> m_Scale;
	};

	class SKTBD_API ChannelNode : public AnimationNode
	{
	public:
		ChannelNode();
		~ChannelNode() override;

		float GetKeyMaxTime() override;

		std::vector<ChannelKey>& GetChannelKeys() { return m_Keys; }

	private:
		std::vector<ChannelKey> m_Keys;
	};


	struct SKTBD_API Animation
	{
	public:
		Animation();
		~Animation();

		virtual const AnimationNode* FindNode(int32_t id);

		virtual const std::map<int32_t, AnimationNode*> GetAnimationNodes() const { return m_AnimationNodes; }
		virtual float GetDuration() const { return m_Duration; }
		virtual float GetStartTime() const { return m_Start; }
		virtual float GetEndTime() const { return m_End; }

		virtual void* GetAnimationData()=0;

	private:
		std::map<int32_t, AnimationNode*> m_AnimationNodes;
		float m_Duration;
		float m_Start;
		float m_End;
		int32_t m_Id;

	};


}