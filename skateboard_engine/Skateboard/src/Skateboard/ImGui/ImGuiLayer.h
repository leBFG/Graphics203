#pragma once
#include "Skateboard/Layer.h"

namespace Skateboard
{
	class ImGuiLayer final : public Layer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer() final override {};

		virtual void OnAttach() final override;
		virtual void OnDetach() final override;
		//virtual void OnUpdate() final override;
		virtual void OnImGuiRender() final override;


		void Begin();
		void End();

	private:
	
	};
}