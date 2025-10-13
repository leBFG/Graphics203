#pragma once

#include "Skateboard/Time/TimeManager.h"
#include "Skateboard/Scene/BaseNativeScript.h"
class PlayerControllerScript : public Skateboard::BaseNativeScript
{
public:
	void OnUpdate(Skateboard::TimeManager* time) override;
	void OnCreate() override;
	void OnDestroy() override;
	void OnHandleInput(Skateboard::TimeManager* time) override;


};

