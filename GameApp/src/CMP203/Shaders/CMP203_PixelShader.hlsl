#include "Structs.hlsli"
#include "Lighting.hlsli"

ConstantBuffer<Constants> PushData : register(b0, space0);
StructuredBuffer<InstanceData> Instances : register(t0, space0);
StructuredBuffer<Light> Lights : register(t1, space0);

cbuffer FRAMEDATA : register(b1, space0)
{
    Frame FrameData;
}

SamplerState g_sampler : register(s0);

float4 main(VertexOutput input) :SV_Target0
{  
    Texture2D texture = ResourceDescriptorHeap[Instances[PushData.instanceNo].textureIDX];
	SamplerState sampler = SamplerDescriptorHeap[Instances[PushData.instanceNo].samplerIDX];
    
    
    //return float4(input.normal.xyz, 1);
    //return saturate(texture.Sample(g_sampler, input.uv) * Instances[PushData.instanceNo].diffuseColour + input.colour);
    //return saturate(sqrt(texture.Sample(g_sampler, input.uv) * texture.Sample(g_sampler, input.uv) * Instances[PushData.instanceNo].diffuseColour * Instances[PushData.instanceNo].diffuseColour + input.colour * input.colour + LetThereBelightSquared(normalize(input.normal), FrameData.CameraMatrix._14_24_34, input.worldpos, Instances[PushData.instanceNo].specular, FrameData.LightCount, Lights)));
    float4 object_colour = pow(texture.Sample(sampler, input.uv), 2) + pow(Instances[PushData.instanceNo].InstanceColour, 2) + pow(input.colour, 2);
    return saturate(sqrt(LetThereBelightSquared(normalize(input.normal), FrameData.CameraMatrix._14_24_34, input.worldpos, Instances[PushData.instanceNo].specular, FrameData.LightCount, Lights, FrameData.AmbientLight) * object_colour));
}