#include "Structs.hlsli"

#define DIRECTIONAL 0
#define POINTLIGHT 1
#define SPOTLIGHT 2

float calculateSpotFalloffFactor(float alpha, Light light)
{
    float falloffFactor = pow((cos(alpha) - cos(light.penumbra / 2)) / (cos(light.umbra / 2) - cos(light.penumbra / 2)), light.falloffPow);
    return falloffFactor;
}

// Calculate lighting intensity based on direction and normal. Combine with light colour.

float calculateLightingIntensity(float3 normal, float3 lightvector)
{
    float intensity = saturate(dot(normal, normalize(lightvector)));
    return intensity;
}

//calculate specular lightting based on view angle Phong model

float calcSpecularIntensity(float SpecularPower, float SpecularStrength, float3 inputView, float3 inputNormal, float3 lightVector)
{
    float3 reflectDir = reflect(-lightVector, inputNormal);
    float spec = pow(max(dot(inputView, reflectDir), 0.0), SpecularPower);
    return spec * SpecularStrength;

   // float3 halfway = normalize(lightVector + inputView);
   // float specularIntensity = pow(max(dot(inputNormal, halfway), 0.0), SpecularPower);
   // return specularIntensity;
    
}

float4 LetThereBelightSquared(float3 normal, float3 viewPos, float3 WorldPos, SpecularData mat, uint LightCount, StructuredBuffer<Light> Lights, float3 AmbientComponent)
{
    float4 LightComponent = float4(0, 0, 0, 0);
    float4 SpecularComponent = float4(0, 0, 0, 0);
    
    float3 viewVec = normalize(viewPos - WorldPos);
    
    
    if (LightCount > 0)
    {
        [loop]
        for (uint i = 0; i < LightCount; i++)
        {
            float3 lightDirection = -normalize(Lights[i].lightDirection);
        
            if (Lights[i].LightType == DIRECTIONAL)
            {
                LightComponent += Lights[i].diffuseColour * Lights[i].diffuseColour * calculateLightingIntensity(normal, lightDirection);
                SpecularComponent += Lights[i].diffuseColour * Lights[i].diffuseColour * calcSpecularIntensity(mat.specularPower,mat.specularStrength, viewVec, normal, lightDirection);
            }
        
            if (Lights[i].LightType == POINTLIGHT)
            {
                float3 lightVector = Lights[i].lightPosition - WorldPos;
                float amplitude = length(lightVector);
                lightVector = normalize(lightVector);

                if (amplitude < Lights[i].attenuation.w)
                {
                    float attune = 1.f / (Lights[i].attenuation.x + Lights[i].attenuation.y * amplitude + Lights[i].attenuation.z * amplitude * amplitude);

                    LightComponent += Lights[i].diffuseColour * Lights[i].diffuseColour * calculateLightingIntensity(normal, normalize(lightVector)) * attune;
                    SpecularComponent += Lights[i].diffuseColour * Lights[i].diffuseColour * calcSpecularIntensity(mat.specularPower, mat.specularStrength, viewVec, normal, lightVector) * attune;
                }
            }
        
            if (Lights[i].LightType == SPOTLIGHT)
            {
                float3 lightVector = Lights[i].lightPosition - WorldPos;
                float amplitude = length(lightVector);
                lightVector = normalize(lightVector);
                

                if (amplitude < Lights[i].attenuation.w)
                {
					//callculate attenuation // tip - very small values of attenuation can cause light to be yoinked beyond 2000 causing all white, amplifying the light
                    float attune = 1.f / (Lights[i].attenuation.x + Lights[i].attenuation.y * amplitude + Lights[i].attenuation.z * amplitude * amplitude);

                    float dot_p = dot(lightDirection, lightVector);
                    float angle = acos(dot_p);

                    if (angle < Lights[i].penumbra / 2.f)
                    {
                        float falloff = calculateSpotFalloffFactor(angle, Lights[i]);
                        LightComponent += Lights[i].diffuseColour * Lights[i].diffuseColour * calculateLightingIntensity(normal, lightVector) * attune * falloff;
                        SpecularComponent += Lights[i].diffuseColour * Lights[i].diffuseColour * calcSpecularIntensity(mat.specularPower, mat.specularStrength, viewVec, normal, lightVector) * attune * falloff;
                    }
                }
            }
        };
        
        //LightComponent /= (1.f + LightComponent);
        //SpecularComponent /= (1.f + SpecularComponent);

    }

    return saturate(pow(float4(AmbientComponent, 0),2) + LightComponent + SpecularComponent);

}