#include "myCamera.h"

void myCamera::Init()
{
	matrices.ProjectionMatrix = glm::perspectiveLH(fov, GraphicsContext::Context->GetClientAspectRatio(), near, far);
	matrices.ViewMatrix = glm::lookAtLH(position, position + forward, up);
}

void myCamera::Update()
{
	float cosY = glm::cos(glm::radians(rotation.y));
	float cosP = glm::cos(glm::radians(rotation.x));
	float cosR = glm::cos(glm::radians(rotation.z));
	float sinY = glm::sin(glm::radians(rotation.y));
	float sinP = glm::sin(glm::radians(rotation.x));
	float sinR = glm::sin(glm::radians(rotation.z));

	forward = {
		sinY* cosP,
		sinP,
		cosP * cosY 
	};
	forward = glm::normalize(forward);

	up = {
		-cosY * sinR - sinY * sinP * cosR,
		cosP * cosR,
		sinY * sinR - sinP * cosR * cosY
	};
	up = glm::normalize(up);

	right = glm::cross(up, forward);

	matrices.ViewMatrix = glm::lookAtLH(position, position + forward, up);
}
