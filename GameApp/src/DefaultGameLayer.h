#pragma once

#include <Skateboard.h>

class DefaultGameLayer final : public Skateboard::Layer
{
public:
	DefaultGameLayer();
	virtual ~DefaultGameLayer() final override;

	virtual void OnEvent(Skateboard::Event& e) final override;

	virtual bool OnHandleInput(Skateboard::TimeManager* time) final override;
	virtual bool OnUpdate(Skateboard::TimeManager* time) final override;
	virtual void OnRender() final override;
	virtual void OnImGuiRender() final override;


private:
	// Scene manager perhaps? Or this is the level in which you may provide one. (a scene loader even)
	std::shared_ptr<Skateboard::Scene> p_Scene;
};