
#ifndef STRUCTS_H
#define STRUCTS_H

struct VertexInput
{
    float3 position : POSITION;
    float3 colour : COLOR;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct VertexOutput
{
    float4 position : SV_Position; // Careful here, it's different!!
    float4 colour : COLOR;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldpos : POSITION;
};

struct Frame
{
    matrix View;
    matrix Projection;
    
    matrix CameraMatrix;
	uint LightCount;
	float3 AmbientLight;
};

struct Constants
{
    uint instanceNo;
};

struct SpecularData
{
    float specularPower;
    float specularStrength;
};

struct InstanceData
{
    matrix World;
    float4 InstanceColour;
    
    int textureIDX;
    int samplerIDX;
    
    SpecularData specular;
};

struct Light
{
    float4 diffuseColour;
    float4 attenuation;

    float3 lightPosition;
    float umbra;

    float3 lightDirection;
    float penumbra;

    float2 padding;
    float falloffPow;
    uint LightType;
};

#endif