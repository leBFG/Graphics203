// includes all the basic structures and most standard library containers used throughout Skateboard
// ZT
#include "sktbdpch.h"
#include "TutorialScene.h"
#include "Skateboard/Platform.h"
#include "Skateboard/Assets/AssetManager.h"

TutorialScene::TutorialScene(const std::string& name): 
	Scene(name)
{
	Renderer.Init();
}

void TutorialScene::OnHandleInput(TimeManager* time)
{
}

void TutorialScene::OnUpdate(TimeManager* time)
{
	Scene::OnUpdate(time);
}

void TutorialScene::drawTriangle()
{
	std::vector<CMP203::Vertex> vertices;
	CMP203::Vertex v0, v1, v2;
	uint32_t indices[3] = { 0, 1, 2 };

	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	v0.Position = float3(-1.0f, 1.0f, 0.0f);
	v1.Position = float3(-1.0f, -1.0f, 0.0f);
	v2.Position = float3(1.0f, -1.0f, 0.0f);
	
	v0.Colour = v1.Colour = v2.Colour = float3(1.f, 1.f, 1.f);
	vertices = { v0, v1, v2 };
	glm::mat4 mRotation, mTranslation, mScale;

	mTranslation = glm::translate(float3(offset));
	mScale = glm::scale(float3(scale, scale, scale));
	mRotation = glm::rotate(glm::radians(rotationAngle), float3(rotationAxis));

	CMP203::InstanceData idTriangle;
	idTriangle.World = mTranslation * mRotation * mScale;
	Renderer.DrawVertices(vertices.data(), vertices.size(), indices, sizeof(indices) / sizeof(uint32), &idTriangle);
}

void TutorialScene::drawSquareTriangleList()
{
	std::vector<CMP203::Vertex> vertices;
	CMP203::Vertex v0, v1, v2, v3;
	uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	v0.Position = float3(-1.0f, 1.0f, 0.0f);
	v1.Position = float3(-1.0f, -1.0, 0.0f);
	v2.Position = float3(1.0f, -1.0f, 0.0f);
	v3.Position = float3(1.0f, 1.0f, 0.0f);

	v0.Colour = v2.Colour = float3(255/255.f, 192/255.f, 203/255.f);
	v2.Colour = v1.Colour = float3(0, 128/255.f, 0);
	vertices = { v0, v1, v2, v3 };

	glm::mat4 mRotation, mTranslation, mScale;

	mTranslation = glm::translate(float3(offset));
	mScale = glm::scale(float3(scale, scale, scale));
	mRotation = glm::rotate(glm::radians(rotationAngle), float3(rotationAxis));

	CMP203::InstanceData idSquareTriangleList;
	idSquareTriangleList.World = mTranslation * mRotation * mScale;
	Renderer.DrawVertices(vertices.data(), vertices.size(), indices, sizeof(indices)/sizeof(uint32), &idSquareTriangleList);
}

void TutorialScene::drawSquareTriangleStrip()
{
	std::vector<CMP203::Vertex> vertices;
	CMP203::Vertex v0, v1, v2, v3;
	uint32_t indices[4] = { 0, 1, 2, 3 };
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	v0.Position = float3(-1.0f, -1.0f, 0.0f);
	v1.Position = float3(1.0f, -1.0f, 0.0f);
	v2.Position = float3(-1.0f, 1.0f, 0.0f);
	v3.Position = float3(1.0f, 1.0f, 0.0f);

	v0.Colour = v2.Colour = float3(255 / 255.f, 192 / 255.f, 203 / 255.f);
	v2.Colour = v1.Colour = float3(0, 128 / 255.f, 0);

	vertices = { v0, v1, v2, v3 };
	glm::mat4 mRotation, mTranslation, mScale;
	mTranslation = glm::translate(float3(offset));
	mScale = glm::scale(float3(scale, scale, scale));
	mRotation = glm::rotate(glm::radians(rotationAngle), float3(rotationAxis));

	CMP203::InstanceData idSquareTriangleStrip;
	idSquareTriangleStrip.World = mTranslation * mRotation * mScale;
	Renderer.DrawVertices(vertices.data(), vertices.size(), indices, sizeof(indices) / sizeof(uint32), &idSquareTriangleStrip);
}

void TutorialScene::drawHexFan()
{
	std::vector<CMP203::Vertex> vertices;
	CMP203::Vertex v0, v1, v2, v3, v4, v5, v6;
	uint32_t indices[13] = { 0, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 1 };
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLEFAN);
 
	v0.Position = float3(0.0f, 0.0f, 0.0f);
	v1.Position = float3(1.0f, 2.0f, 0.0f);
	v2.Position = float3(-1.0f, 2.0f, 0.0f);
	v3.Position = float3(-2.0f, 0.0f, 0.0f);
	v4.Position = float3(-1.0f, -2.0f, 0.0f);
	v5.Position = float3(1.0f, -2.0f, 0.0f);
	v6.Position = float3(2.0f, 0.0f, 0.0f);


	v0.Colour = v1.Colour = v2.Colour = v3.Colour = v4.Colour = v5.Colour = v6.Colour = float3(1.f, 1.f, 1.f);
	vertices = { v0, v1, v2, v3, v4, v5, v6 };

	glm::mat4 mRotation, mTranslation, mScale;
	mTranslation = glm::translate(float3(offset));
	mScale = glm::scale(float3(scale, scale, scale));
	mRotation = glm::rotate(glm::radians(rotationAngle), float3(rotationAxis));

	CMP203::InstanceData idHexFan;
	idHexFan.World = mTranslation * mRotation * mScale;
	Renderer.DrawVertices(vertices.data(), vertices.size(), indices, sizeof(indices)/sizeof(uint32), &idHexFan);
}

void TutorialScene::drawCube()
{
	std::vector<CMP203::Vertex> vertices;
	std::vector<uint32_t> indices;

	// Cube vertice positions
	float3 bottomLeft = { -1.0f, -1.0f, 1.0f };			// 0
	float3 bottomRight = { 1.0f, -1.0f, 1.0f };			// 1
	float3 topLeft = { -1.0f, 1.0f, 1.0f };				// 2
	float3 topRight = { 1.0f, 1.0f, 1.0f };				// 3
	float3 backBottomLeft = { -1.0f, -1.0f, -1.0f };	// 4
	float3 backBottomRight = { 1.0f, -1.0f, -1.0f };	// 5
	float3 backTopLeft = { -1.0f, 1.0f, -1.0f };		// 6
	float3 backTopRight = { 1.0f, 1.0f, -1.0f };		// 7

	// Colours
	float3 red = { 1.f, 0.f, 0.f };
	float3 green = { 0.f, 1.f, 0.f };
	float3 blue = { 0.f, 0.f, 1.f };
	float3 yellow = { 1.f, 1.f, 0.f };
	float3 white = { 1.f, 1.f, 1.f };
	float3 darkBlue = { 0.f, 0.f, 0.5f };
	
	// Front face
	vertices.push_back({ bottomLeft, red });		// 0
	vertices.push_back({ bottomRight, red });		// 1
	vertices.push_back({ topLeft, red });			// 2
	vertices.push_back({ topRight, red });			// 3

	// Right face
	vertices.push_back({ bottomRight, green });		// 4
	vertices.push_back({ backBottomRight, green });	// 5
	vertices.push_back({ topRight, green });		// 6
	vertices.push_back({ backTopRight, green });	// 7

	// Back face
	vertices.push_back({ backBottomRight, blue });	// 8
	vertices.push_back({ backBottomLeft, blue });	// 9
	vertices.push_back({ backTopRight, blue });		// 10
	vertices.push_back({ backTopLeft, blue });		// 11

	// Left face
	vertices.push_back({ backBottomLeft, yellow });	// 12
	vertices.push_back({ bottomLeft, yellow });		// 13
	vertices.push_back({ backTopLeft, yellow });	// 14
	vertices.push_back({ topLeft, yellow });		// 15

	// Top face
	vertices.push_back({ topLeft, white });			// 16
	vertices.push_back({ topRight, white });		// 17
	vertices.push_back({ backTopLeft, white });		// 18
	vertices.push_back({ backTopRight, white });	// 19

	// Bottom face
	vertices.push_back({ bottomLeft, darkBlue });		// 20
	vertices.push_back({ bottomRight, darkBlue });		// 21
	vertices.push_back({ backBottomLeft, darkBlue });	// 22
	vertices.push_back({ backBottomRight, darkBlue });	// 23

	// Indices
	// Front face
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);
	indices.push_back(2);

	// Right face
	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(5);
	indices.push_back(7);
	indices.push_back(6);

	// Back face
	indices.push_back(8);
	indices.push_back(9);
	indices.push_back(10);
	indices.push_back(9);
	indices.push_back(11);
	indices.push_back(10);

	// Left face
	indices.push_back(12);
	indices.push_back(13);
	indices.push_back(14);
	indices.push_back(13);
	indices.push_back(15);
	indices.push_back(14);

	// Top face
	indices.push_back(16);
	indices.push_back(17);
	indices.push_back(18);
	indices.push_back(17);
	indices.push_back(19);
	indices.push_back(18);

	// Bottom face
	indices.push_back(20);
	indices.push_back(21);
	indices.push_back(22);
	indices.push_back(21);
	indices.push_back(23);
	indices.push_back(22);


	glm::mat4 mRotation, mTranslation, mScale;
	mTranslation = glm::translate(float3(offset));
	mScale = glm::scale(float3(scale, scale, scale));
	mRotation = glm::rotate(glm::radians(rotationAngle), float3(rotationAxis));

	CMP203::InstanceData idCube;
	idCube.World = mTranslation * mRotation * mScale;
	Renderer.DrawVertices(vertices.data(), vertices.size(), indices.data(), indices.size(), &idCube);
}

void TutorialScene::drawDisc() // NOT WORKING
{
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLEFAN);

	float interval = 2.f * SKTBD_PI / discSegments;
	float theta = 0.f;

	std::vector<CMP203::Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int i = 0; 1 < discSegments; i++)
	{
		CMP203::Vertex v0, v1, v2;
		v0.Position = { 0, 0, 0 };
		v1.Position = { radius * glm::cos(theta), radius * glm::sin(theta), 0 };
		theta += interval;
		if (i < (discSegments - 1))
		{
			v2.Position = { radius * glm::cos(theta), radius * glm::sin(theta), 0 };
		}
		else
		{
			v2.Position = { radius * glm::cos(0), radius * glm::sin(0), 0 };
		}

		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);
		indices.push_back(0 + (i * 3));
		indices.push_back(1 + (i * 3));
		indices.push_back(2 + (i * 3));
	}

	glm::mat4 mRotation, mTranslation, mScale;
	mTranslation = glm::translate(float3(offset));
	mScale = glm::scale(float3(scale, scale, scale));
	mRotation = glm::rotate(glm::radians(rotationAngle), float3(rotationAxis));

	CMP203::InstanceData idDrawDisc;
	idDrawDisc.World = mTranslation * mRotation * mScale;
	Renderer.DrawVertices(vertices.data(), vertices.size(), indices.data(), indices.size(), &idDrawDisc);
}

void TutorialScene::drawRobotArm()
{
	// Upper Arm
	std::vector<CMP203::Vertex> verticesUpperArm;
	CMP203::Vertex vUA0, vUA1, vUA2, vUA3;
	uint32_t indicesUpperArm[4] = { 0, 1, 2, 3 };
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	glm::float3 armColour = { 1.f, 1.f, 1.f };

	vUA0.Position = float3(-0.1f, -0.1f, 0.0f);
	vUA1.Position = float3(0.1f, -0.1f, 0.0f);
	vUA2.Position = float3(-0.1f, 0.1f, 0.0f);
	vUA3.Position = float3(0.1f, 0.1f, 0.0f);

	vUA0.Colour = vUA1.Colour = vUA2.Colour = vUA3.Colour = armColour;

	verticesUpperArm = { vUA0, vUA1, vUA2, vUA3 };
	glm::mat4 mupperRotation, mupperTranslation, mupperScale;
	mupperTranslation = glm::translate(float3(armOffset));
	mupperScale = glm::scale(float3(armScale, armScale, armScale));
	mupperRotation = glm::rotate(glm::radians(armRotationAngle), float3(armRotationAxis));

	CMP203::InstanceData idUpperArm;
	idUpperArm.World = mupperTranslation * mupperRotation * mupperScale;
	Renderer.DrawVertices(verticesUpperArm.data(), verticesUpperArm.size(), indicesUpperArm, sizeof(indicesUpperArm) / sizeof(uint32), &idUpperArm);

	// Lower Arm
	std::vector<CMP203::Vertex> verticesLowerArm;
	CMP203::Vertex vLA0, vLA1, vLA2, vLA3;
	uint32_t indicesLowerArm[4] = { 0, 1, 2, 3 };
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	vLA0.Position = float3(-0.1f, 0.1f, 0.0f);
	vLA1.Position = float3(0.1f, 0.1f, 0.0f);
	vLA2.Position = float3(-0.1f, 0.3f, 0.0f);
	vLA3.Position = float3(0.1f, 0.3f, 0.0f);

	vLA0.Colour = vLA1.Colour = vLA2.Colour = vLA3.Colour = armColour;

	verticesLowerArm = { vLA0, vLA1, vLA2, vLA3 };
	glm::mat4 mlowerRotation, mlowerTranslation, mlowerScale;
	mlowerTranslation = glm::translate(float3(lowerArmOffset));
	mlowerScale = glm::scale(float3(lowerArmScale, lowerArmScale, lowerArmScale));
	mlowerRotation = glm::rotate(glm::radians(lowerArmRotationAngle), float3(lowerArmRotationAxis));
	CMP203::InstanceData idLowerArm;
	idLowerArm.World = idUpperArm.World / mupperScale * mlowerTranslation * mlowerRotation * mlowerScale;
	Renderer.DrawVertices(verticesLowerArm.data(), verticesLowerArm.size(), indicesLowerArm, sizeof(indicesLowerArm) / sizeof(uint32), &idLowerArm);

	// Hand
	std::vector<CMP203::Vertex> verticesHand;
	CMP203::Vertex vH0, vH1, vH2, vH3;
	uint32_t indicesHand[4] = { 0, 1, 2, 3 };
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	vH0.Colour = vH1.Colour = vH2.Colour = vH3.Colour = armColour;

	vH0.Position = float3(-0.1f, 0.3f, 0.0f);
	vH1.Position = float3(0.1f, 0.3f, 0.0f);
	vH2.Position = float3(-0.1f, 0.5f, 0.0f);
	vH3.Position = float3(0.1f, 0.5f, 0.0f);

	verticesHand = { vH0, vH1, vH2, vH3 };
	glm::mat4 mhandRotation, mhandTranslation, mhandScale;
	mhandTranslation = glm::translate(float3(handOffset));
	mhandScale = glm::scale(float3(handScale, handScale, handScale));
	mhandRotation = glm::rotate(glm::radians(handRotationAngle), float3(handRotationAxis));
	CMP203::InstanceData idHand;
	idHand.World = idLowerArm.World / mlowerScale * mhandTranslation * mhandRotation * mhandScale;
	Renderer.DrawVertices(verticesHand.data(), verticesHand.size(), indicesHand, sizeof(indicesHand) / sizeof(uint32), &idHand);
}

void TutorialScene::OnRender()
{
	Renderer.Begin();

	//drawTriangle();
	//drawSquareTriangleList();
	//drawSquareTriangleStrip();
	//drawHexFan();
	drawCube();
	//drawDisc(); // NOT WORKING - won't even start :')
	//drawRobotArm();

	Renderer.End();
}

void TutorialScene::OnImGuiRender()
{
	ImGui::Begin("ImGui");//creates new window

	ImGui::Text("Hello CMP203!");
	ImGui::Text("FPS: %f", Platform::GetTimeManager()->FPS());
	ImGui::Text("mouse position X: %d, Y: %d", Input::GetMousePos().x, Input::GetMousePos().y);
	if (ImGui::Checkbox("wireframe", &bWireframe))
	{
		if (bWireframe)
			Renderer.SetPipelineFlags(CMP203::PipelineFlags::WIREFRAME);
		else
			Renderer.UnsetPipelineFlags(CMP203::PipelineFlags::WIREFRAME);

	}
	ImGui::Separator();
	ImGui::SliderFloat3("Position offset", (float*)&offset, -5.f, 5.f);
	ImGui::Separator();
	ImGui::SliderFloat3("Rotation axis", (float*)&rotationAxis, -1.f, 1.f);
	ImGui::Separator();
	ImGui::SliderFloat("Rotation Angle", (float*)&rotationAngle, 180.f, -180.f);
	ImGui::Separator();
	ImGui::SliderFloat("Scale", (float*)&scale, 0.1f, 100.f);
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text("Robot Arm Controls");
	ImGui::Separator();
	ImGui::SliderFloat3("Upper Arm Position offset", (float*)&armOffset, -5.f, 5.f);
	ImGui::Separator();
	ImGui::SliderFloat3("Upper Arm Rotation axis", (float*)&armRotationAxis, -1.f, 1.f);
	ImGui::Separator();
	ImGui::SliderFloat("Upper Arm Rotation Angle", (float*)&armRotationAngle, 180.f, -180.f);
	ImGui::Separator();
	ImGui::SliderFloat("Upper Arm Scale", (float*)&armScale, 0.1f, 100.f);
	ImGui::Separator();
	ImGui::SliderFloat3("Lower Arm Position offset", (float*)&lowerArmOffset, -5.f, 5.f);
	ImGui::Separator();
	ImGui::SliderFloat3("Lower Arm Rotation axis", (float*)&lowerArmRotationAxis, -1.f, 1.f);
	ImGui::Separator();
	ImGui::SliderFloat("Lower Arm Rotation Angle", (float*)&lowerArmRotationAngle, 180.f, -180.f);
	ImGui::Separator();
	ImGui::SliderFloat("Lower Arm Scale", (float*)&lowerArmScale, 0.1f, 100.f);
	ImGui::Separator();
	ImGui::SliderFloat3("Hand Position offset", (float*)&handOffset, -5.f, 5.f);
	ImGui::Separator();
	ImGui::SliderFloat3("Hand Rotation axis", (float*)&handRotationAxis, -1.f, 1.f);
	ImGui::Separator();
	ImGui::SliderFloat("Hand Rotation Angle", (float*)&handRotationAngle, 180.f, -180.f);
	ImGui::Separator();
	ImGui::SliderFloat("Hand Scale", (float*)&handScale, 0.1f, 100.f);
	ImGui::Separator();
	ImGui::End(); 
}
