#pragma once  
#include "Skateboard/Scene/Scene.h"  
#include "CMP203/Renderer203.h"  
#include "../myCamera.h"

class TutorialScene : public Skateboard::Scene {  
public:  
   explicit TutorialScene(const std::string& name);  

   TutorialScene() = delete;  

   virtual void OnHandleInput(Skateboard::TimeManager* time) override;  
   virtual void OnUpdate(Skateboard::TimeManager* time) override;  
   virtual void OnRender() override;  

   virtual void OnImGuiRender() override;  
private:  
   CMP203::Renderer203 Renderer;  
   bool bWireframe = false;  

   void drawTriangle();  
   void drawSquareTriangleList();  
   void drawSquareTriangleStrip();  
   void drawHexFan();  
   void drawCube();  
   void drawDisc();  
   void drawRobotArm();  

   glm::float3 offset = { 0, 0, 0 };  
   glm::float3 rotationAxis = { 0, 0, 1 };  
   float rotationAngle = 0;  
   float scale = 5;  

   glm::float3 armOffset = { 0, 0, 0 };  
   glm::float3 armRotationAxis = { 0, 0, 1 };  
   float armRotationAngle = 0;  
   float armScale = 1;  

   glm::float3 lowerArmOffset = { 0, 0, 0 };  
   glm::float3 lowerArmRotationAxis = { 0, 0, 1 };  
   float lowerArmRotationAngle = 0;  
   float lowerArmScale = 1;  

   glm::float3 handOffset = { 0, 0, 0 };  
   glm::float3 handRotationAxis = { 0, 0, 1 };  
   float handRotationAngle = 0;  
   float handScale = 1;  

   float discSegments = 3.f;  
   float radius = 2;  

   myCamera myCamera;
   int2 mousePosOld = { 0, 0 };
};
