#pragma once
#include "CMP203/Renderer203.h"

class myCamera
{
public:
	CMP203::CameraData matrices;
	float3 rotation = { 0, 0, 0 };
	float3 position = { 0, 0, 0 };
	void Init();
	void Update();

	float3 forward = { 0, 0, 1 };
	float3 up = { 0, 1, 0 };
	float3 right = glm::cross(up, forward);

	float fov = glm::radians(60.0f);
	float near = 0.1f;
	float far = 100.0f;

	float speed = 40.0f;

	//float3 getCamPos() { return position; };
	//float3 getCamRot() { return rotation; };
	//void setCamPos(float3 newPos);
	//void setCamRot();
};

