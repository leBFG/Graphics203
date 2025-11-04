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
	Renderer.SetAmbientLight(float3(0.4f, 0.3f, 0.3f));

	CMP203::Light DirectionalLight(CMP203::LightType::LightDirectional);
	CMP203::Light PointLight(CMP203::LightType::LightPoint);
	CMP203::Light SpotLight(CMP203::LightType::LightSpot);

	DirectionalLight.Type = CMP203::LightType::LightDirectional;
	DirectionalLight.DiffuseColour = float4(1.f, 1.f, 1.f, 1.f);
	DirectionalLight.LightDirection = float3(0.f, -1.f, 0.f);

	SpotLight.Type = CMP203::LightType::LightSpot;
	SpotLight.DiffuseColour = float4(1.f, 1.f, 0.f, 1.f);
	SpotLight.LightPosition = float3(0.f, -1.f, 0.f);
	SpotLight.LightDirection = float3(0.f, -1.f, 0.f);
	SpotLight.ConstantAttenuation = 0.05f;
	SpotLight.LinearAttenuation = 0.01f;
	SpotLight.SquareAttenuation = 0.001f;
	SpotLight.InnerCone = glm::radians(30.f);
	SpotLight.OuterCone = glm::radians(90.f);
	SpotLight.FalloffPower = 2.f;

	PointLight.Type = CMP203::LightType::LightPoint;
	PointLight.DiffuseColour = float4(1.f, 0.f, 0.f, 1.f);
	PointLight.LightPosition = float3(0.f, -1.f, 0.f);
	PointLight.ConstantAttenuation = 0.05f;
	PointLight.LinearAttenuation = 0.01f;
	PointLight.SquareAttenuation = 0.001f;

	SceneLights.push_back(DirectionalLight);	// Pos 0
	SceneLights.push_back(PointLight);			// Pos 1
	SceneLights.push_back(SpotLight);			// Pos 2
	Renderer.SetLights(SceneLights);

	myCamera.Init();

	SamplerDesc SamplerDesc_Mirror;
	SamplerDesc SamplerDesc_Clamp;
	SamplerDesc SamplerDesc_Repeat;

	SamplerDesc_Mirror = CMP203::LinearSampler(SamplerMode_Mirror, SamplerMode_Wrap);
	SamplerDesc_Clamp = CMP203::LinearSampler(SamplerMode_Clamp, SamplerMode_Clamp);
	SamplerDesc_Repeat = CMP203::LinearSampler(SamplerMode_Wrap, SamplerMode_Wrap);

	SamplerMirror = ResourceFactory::CreateSampler(SamplerDesc_Mirror, L"MirrorU");
	SamplerClamp = ResourceFactory::CreateSampler(SamplerDesc_Clamp, L"ClampU");
	SamplerRepeat = ResourceFactory::CreateSampler(SamplerDesc_Repeat, L"RepeatU");

	AssetManager::LoadTexture(L"assets/stone", "Stone");
	AssetManager::LoadTexture(L"assets/Dice", "Dice");
	AssetManager::LoadTexture(L"assets/crate", "Crate");
	AssetManager::LoadTexture(L"assets/fsjal", "WeirdFace");
}

void TutorialScene::OnHandleInput(TimeManager* time)
{
	if (Input::IsKeyDown(Keys::sc_w))
	{
		myCamera.position += myCamera.forward * myCamera.speed * time->DeltaTime();
		myCamera.Update();
	}
	else if (Input::IsKeyDown(Keys::sc_s))
	{
		myCamera.position -= myCamera.forward * myCamera.speed * time->DeltaTime();
		myCamera.Update();
	}

	if (Input::IsKeyDown(Keys::sc_d))
	{
		myCamera.position += myCamera.right * myCamera.speed * time->DeltaTime();
		myCamera.Update();
	}
	else if (Input::IsKeyDown(Keys::sc_a))
	{
		myCamera.position -= myCamera.right * myCamera.speed * time->DeltaTime();
		myCamera.Update();
	}

	if (Input::IsMouseButtonPressed(MouseBtn::mb_RightButton))	{
		Input::SetMouseVisible(false);
	}
	else if (Input::IsMouseButtonReleased(MouseBtn::mb_RightButton))	{
		Input::SetMouseVisible(true);
	}

	if (Input::IsMouseButtonDown(MouseBtn::mb_RightButton))
	{
		int2 window_centre = (int2)Input::GetWindowSize() / 2;
		int2 mouse_deltas = Input::GetMousePos() - mousePosOld;
		
		myCamera.rotation.y += mouse_deltas.x * time->DeltaTime() * myCamera.speed / 2;
		myCamera.rotation.x -= mouse_deltas.y * time->DeltaTime() * myCamera.speed / 2;
		myCamera.Update();
		Input::SetMousePos(window_centre.x, window_centre.y);
	}

	mousePosOld = Input::GetMousePos();
}

void TutorialScene::OnUpdate(TimeManager* time)
{
	Scene::OnUpdate(time);
	Renderer.SetCameraData(myCamera.matrices);
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

void TutorialScene::drawRoom()
{
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

	// Texture vertice positions
	float2 textPosTopLeft = { 0, 0 };
	float2 textPosBottomLeft = { 0, 1 };
	float2 textPosBottomRight = { 1, 1 };
	float2 textPosTopRight = { 1, 0 };
	
	
	// Front face
	vertices.push_back({ bottomLeft, red, textPosBottomLeft * rScale, float3{ 0.f, 0.f, -1.f } });		// 0
	vertices.push_back({ bottomRight, red, textPosBottomRight * rScale, float3{ 0.f, 0.f, -1.f } });		// 1
	vertices.push_back({ topLeft, red, textPosTopLeft * rScale, float3{ 0.f, 0.f, -1.f } });			// 2
	vertices.push_back({ topRight, red, textPosTopRight * rScale, float3{ 0.f, 0.f, -1.f } });			// 3

	// Right face
	vertices.push_back({ bottomRight, green, textPosBottomLeft * rScale, float3{ -1.f, 0.f, 0.f } });		// 4
	vertices.push_back({ backBottomRight, green, textPosBottomRight * rScale, float3{ -1.f, 0.f, 0.f } });	// 5
	vertices.push_back({ topRight, green, textPosTopLeft * rScale, float3{ -1.f, 0.f, 0.f } });		// 6
	vertices.push_back({ backTopRight, green, textPosTopRight * rScale, float3{ -1.f, 0.f, 0.f } });	// 7

	// Back face
	vertices.push_back({ backBottomRight, blue, textPosBottomLeft * rScale, float3{ 0.f, 0.f, 1.f } });	// 8
	vertices.push_back({ backBottomLeft, blue, textPosBottomRight * rScale, float3{ 0.f, 0.f, 1.f } });	// 9
	vertices.push_back({ backTopRight, blue, textPosTopLeft * rScale, float3{ 0.f, 0.f, 1.f } });		// 10
	vertices.push_back({ backTopLeft, blue, textPosTopRight * rScale, float3{ 0.f, 0.f, 1.f } });		// 11

	// Left face
	vertices.push_back({ backBottomLeft, yellow, textPosBottomLeft * rScale, float3{ 1.f, 0.f, 0.f } });	// 12
	vertices.push_back({ bottomLeft, yellow, textPosBottomRight * rScale, float3{ 1.f, 0.f, 0.f } });		// 13
	vertices.push_back({ backTopLeft, yellow, textPosTopLeft * rScale, float3{ 1.f, 0.f, 0.f } });	// 14
	vertices.push_back({ topLeft, yellow, textPosTopRight * rScale, float3{ 1.f, 0.f, 0.f } });		// 15

	// Top face
	vertices.push_back({ topLeft, white, textPosBottomLeft * rScale, float3{ 0.f, -1.f, 0.f } });			// 16
	vertices.push_back({ topRight, white, textPosBottomRight * rScale, float3{ 0.f, -1.f, 0.f } });		// 17
	vertices.push_back({ backTopLeft, white, textPosTopLeft * rScale, float3{ 0.f, -1.f, 0.f } });		// 18
	vertices.push_back({ backTopRight, white, textPosTopRight * rScale, float3{ 0.f, -1.f, 0.f } });	// 19

	// Bottom face
	vertices.push_back({ backBottomLeft, darkBlue, textPosBottomLeft * rScale, float3{ 0.f, 1.f, 0.f } });		// 20
	vertices.push_back({ backBottomRight, darkBlue, textPosBottomRight * rScale, float3{ 0.f, 1.f, 0.f } });		// 21
	vertices.push_back({ bottomLeft, darkBlue, textPosTopLeft * rScale, float3{ 0.f, 1.f, 0.f } });	// 22
	vertices.push_back({ bottomRight, darkBlue, textPosTopRight * rScale, float3{ 0.f, 1.f, 0.f } });	// 23


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
	indices.push_back(20); //22
	indices.push_back(21); //23
	indices.push_back(22); //21
	indices.push_back(21); //22
	indices.push_back(23); //21
	indices.push_back(22); //20

	glm::mat4 mRotation, mTranslation, mScale;
	mTranslation = glm::translate(float3(rOffset));
	mScale = glm::scale(float3(rScale, 3, rScale));
	mRotation = glm::rotate(glm::radians(rRotationAngle), float3(rRotationAxis));

	CMP203::InstanceData idRoom;
	idRoom.World = mTranslation * mRotation * mScale;
	idRoom.TextureIndex = AssetManager::GetTexture("Stone")->GetViewIndex();
	idRoom.SamplerIndex = SamplerMirror->GetSamplerIndex();
	Renderer.DrawVertices(vertices.data(), vertices.size(), indices.data(), indices.size(), &idRoom);
}

void TutorialScene::drawCube()
{
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

	// Texture vertice positions
	float2 textPosTopLeft = { 0, 0 };
	float2 textPosTopRight = { 1, 0 };
	float2 textPosBottomLeft = { 0, 1 };
	float2 textPosBottomRight = { 1, 1 };


	// Front face
	vertices.push_back({ bottomLeft, white, textPosBottomRight, float3{ 0.f, 0.f, 1.f } });		// 0
	vertices.push_back({ bottomRight, white, textPosBottomLeft, float3{ 0.f, 0.f, 1.f } });		// 1
	vertices.push_back({ topLeft, white, textPosTopRight, float3{ 0.f, 0.f, 1.f } });			// 2
	vertices.push_back({ topRight, white, textPosTopLeft, float3{ 0.f, 0.f, 1.f } });			// 3

	// Right face
	vertices.push_back({ bottomRight, white, textPosBottomRight, float3{ 1.f, 0.f, 0.f } });		// 4
	vertices.push_back({ backBottomRight, white, textPosBottomLeft, float3{ 1.f, 0.f, 0.f } });	// 5
	vertices.push_back({ topRight, white, textPosTopRight, float3{ 1.f, 0.f, 0.f } });		// 6
	vertices.push_back({ backTopRight, white, textPosTopLeft, float3{ 1.f, 0.f, 0.f } });	// 7

	// Back face
	vertices.push_back({ backBottomRight, white, textPosBottomRight, float3{ 0.f, 0.f, -1.f } });	// 8
	vertices.push_back({ backBottomLeft, white, textPosBottomLeft, float3{ 0.f, 0.f, -1.f } });	// 9
	vertices.push_back({ backTopRight, white, textPosTopRight, float3{ 0.f, 0.f, -1.f } });		// 10
	vertices.push_back({ backTopLeft, white, textPosTopLeft, float3{ 0.f, 0.f, -1.f } });		// 11

	// Left face
	vertices.push_back({ backBottomLeft, white, textPosBottomRight, float3{ -1.f, 0.f, 0.f } });	// 12
	vertices.push_back({ bottomLeft, white, textPosBottomLeft, float3{ -1.f, 0.f, 0.f } });		// 13
	vertices.push_back({ backTopLeft, white, textPosTopRight, float3{ -1.f, 0.f, 0.f } });	// 14
	vertices.push_back({ topLeft, white, textPosTopLeft, float3{ -1.f, 0.f, 0.f } });		// 15

	// Top face
	vertices.push_back({ topLeft, white, textPosBottomRight, float3{ 0.f, 1.f, 0.f } });			// 16
	vertices.push_back({ topRight, white, textPosBottomLeft, float3{ 0.f, 1.f, 0.f } });		// 17
	vertices.push_back({ backTopLeft, white, textPosTopRight, float3{ 0.f, 1.f, 0.f } });		// 18
	vertices.push_back({ backTopRight, white, textPosTopLeft, float3{ 0.f, 1.f, 0.f } });	// 19

	// Bottom face
	vertices.push_back({ bottomLeft, white, textPosBottomLeft, float3{ 0.f, -1.f, 0.f } });		// 20
	vertices.push_back({ bottomRight, white, textPosBottomRight, float3{ 0.f, -1.f, 0.f } });		// 21
	vertices.push_back({ backBottomLeft, white, textPosTopLeft, float3{ 0.f, -1.f, 0.f } });	// 22
	vertices.push_back({ backBottomRight, white, textPosTopRight, float3{ 0.f, -1.f, 0.f } });	// 23


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
	indices.push_back(22);
	indices.push_back(23);
	indices.push_back(21);
	indices.push_back(22);
	indices.push_back(21);
	indices.push_back(20);

	std::reverse(indices.begin(), indices.end());

	glm::mat4 mRotation, mTranslation, mScale;
	mTranslation = glm::translate(float3(offset));
	mScale = glm::scale(float3(scale, scale, scale));
	mRotation = glm::rotate(glm::radians(rotationAngle), float3(rotationAxis));

	CMP203::InstanceData idCube;
	idCube.World = mTranslation * mRotation * mScale;
	idCube.TextureIndex = AssetManager::GetTexture("Crate")->GetViewIndex();
	idCube.SamplerIndex = SamplerClamp->GetSamplerIndex();
	Renderer.DrawVertices(vertices.data(), vertices.size(), indices.data(), indices.size(), &idCube);
}

void TutorialScene::drawDisc() // NOT WORKING
{
	Renderer.SetTopology(SKTBD_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float interval = 2.f * SKTBD_PI / discSegments;
	float theta = 0.f;

	std::vector<CMP203::Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int i = 0; i < discSegments; i++)
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
	Renderer.SetPipelineFlags(CMP203::PipelineFlags::LIT);
	Renderer.SetDrawDebugNormals(true);

	//drawTriangle();
	//drawSquareTriangleList();
	//drawSquareTriangleStrip();
	//drawHexFan();
	drawRoom();
	//drawCube();
	drawDisc();
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
	/*if (ImGui::Checkbox("Lighting", &bLighting))
	{
		if (bWireframe)
			Renderer.SetPipelineFlags(CMP203::PipelineFlags::LIT);
		else
			Renderer.UnsetPipelineFlags(CMP203::PipelineFlags::LIT);
	}*/
	ImGui::Separator();
	// General Controls
	ImGui::Text("General Controls");
	ImGui::SliderFloat3("Position offset", (float*)&offset, -5.f, 5.f);
	ImGui::Separator();
	ImGui::SliderFloat3("Rotation axis", (float*)&rotationAxis, -1.f, 1.f);
	ImGui::Separator();
	ImGui::SliderFloat("Rotation Angle", (float*)&rotationAngle, 180.f, -180.f);
	ImGui::Separator();
	ImGui::SliderFloat("Scale", (float*)&scale, 0.1f, 90.f);
	ImGui::Separator();
	//Disc Controls
	ImGui::Text("Disc Controls");
	ImGui::SliderFloat("Radius", (float*)&radius, 0.1f, 90.f);
	ImGui::SliderFloat("Segments", (float*)&discSegments, 0.1f, 90.f);

	// Room controls
	ImGui::Text("Room Controls");
	ImGui::SliderFloat3("Room Position offset", (float*)&rOffset, -5.f, 5.f);
	ImGui::Separator();
	ImGui::SliderFloat3("Room Rotation axis", (float*)&rRotationAxis, -1.f, 1.f);
	ImGui::Separator();
	ImGui::SliderFloat("Room Rotation Angle", (float*)&rRotationAngle, 180.f, -180.f);
	ImGui::Separator();
	ImGui::SliderFloat("Room Scale", (float*)&rScale, 0.1f, 90.f);
	ImGui::Separator();
	// Robot Arm Controls
	/*ImGui::Text("Robot Arm Controls");
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
	ImGui::Separator();*/
	// Camera ImGui Controls
	ImGui::Text("Camera Controls");
	bool sliderChanged1 = false;
	bool sliderChanged2 = false;
	bool sliderChanged3 = false;
	ImGui::Separator();
	sliderChanged1 = ImGui::SliderFloat3("Camera Position", (float*)&myCamera.position, -50.f, 50.f);
	ImGui::Separator();
	sliderChanged2 = ImGui::SliderFloat("Camera Rotation X", (float*)&myCamera.rotation.x, -180.f, 180.f);
	ImGui::Separator();
	sliderChanged3 = ImGui::SliderFloat("Camera Rotation Y", (float*)&myCamera.rotation.y, -180.f, 180.f);
	ImGui::Separator();
	ImGui::Separator();
	bool pointLightSliderChanged = false;
	bool spotLightSliderChanged = false;
	pointLightSliderChanged = ImGui::SliderFloat3("Point Light Position", (float*)&SceneLights[1].LightPosition, -50.f, 50.f);
	ImGui::Separator();
	spotLightSliderChanged = ImGui::SliderFloat3("Spot Light Position", (float*)&SceneLights[2].LightPosition, -50.f, 50.f);

	ImGui::End(); 

	if (sliderChanged1)
		myCamera.Update();
	if (sliderChanged2)
		myCamera.Update();
	if (sliderChanged3)
		myCamera.Update();

	if (pointLightSliderChanged)
		Renderer.SetLights(SceneLights);
	if (spotLightSliderChanged)
		Renderer.SetLights(SceneLights);
}
