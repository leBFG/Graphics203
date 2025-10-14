#pragma once
#include "Skateboard/Time/TimeManager.h"
#include "CMP203/Renderer203.h"
#include "TransformComponent.h"

class GameObject {
public:
    GameObject();
    virtual ~GameObject();

    virtual void Update(Skateboard::TimeManager* time);
    virtual void Render(CMP203::Renderer203& renderer);


protected:
};
