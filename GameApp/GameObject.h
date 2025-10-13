#pragma once
#include "Skateboard/Time/TimeManager.h"
#include "CMP203/Renderer203.h"
#include "TransformComponent.h"

class GameObject {
public:
    GameObject() = default;
    virtual ~GameObject() = default;

    virtual void Update(Skateboard::TimeManager* time) {}
    virtual void Render(CMP203::Renderer203& renderer) = 0;

    TransformComponent& GetTransform() { return TransformData; }

protected:
    TransformComponent TransformData;
};
