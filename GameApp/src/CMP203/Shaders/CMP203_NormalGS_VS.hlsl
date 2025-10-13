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

    output.position = float4(input.position, 1);
    output.worldpos = output.position;
    output.uv = input.uv;
    output.colour = float4(input.colour, 1);
    output.normal = input.normal;

    return output;
}