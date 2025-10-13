#include "Structs.hlsli"

ConstantBuffer<Constants> PushData : register(b0, space0);
StructuredBuffer<InstanceData> Instances : register(t0, space0);

cbuffer FRAMEDATA : register(b1, space0)
{
    Frame FrameData;
}

VertexOutput main(in VertexInput input)
{
	VertexOutput output;

    output.position = mul(Instances[PushData.instanceNo].World, float4(input.position, 1.f));
    output.worldpos = output.position;
    output.position = mul(FrameData.View, output.position);
    output.position = mul(FrameData.Projection, output.position);
	output.uv = input.uv;
    output.colour = float4(input.colour,0);
    
    //output.normal = input.normal;
    
    output.normal = mul((float3x3)(Instances[PushData.instanceNo].World), input.normal);

	return output;
}

