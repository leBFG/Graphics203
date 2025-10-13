
struct PixelInput
{
    float4 position : SV_Position; // Careful here, it's different!!
    float3 normal : NORMAL;
};


float4 main(PixelInput input) : SV_TARGET
{
    return float4((input.normal + 1) / 2.f, 1.0f);
}