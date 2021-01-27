struct PixelShaderInput
{
    float4 ViewSpacePosition : POSITION;
    float3 ViewSpaceNormal   : NORMAL;
    float2 TexCoord          : TEXCOORD;
};

struct Material
{
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
    float4 Ambient;
    float4 Reflectance;
    
    float Opacity; 
    float SpecularPower;
    float IndexOfRefraction; 
    float BumpIntensity; 
    float AlphaThreshold; 
    
    bool HasAmbientTexture;
    bool HasEmissiveTexture;
    bool HasDiffuseTexture;
    bool HasSpecularTexture;
    bool HasSpecularPowerTexture;
    bool HasNormalTexture;
    bool HasBumpTexture;
    bool HasOpacityTexture;
    
    float3 Padding; 
};

struct PointLight
{
    float4 WorldSpacePosition; 
    float4 ViewSpacePosition; 
    float4 Color;
  
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;
    float Padding;
};

struct SpotLight
{
    float4 WorldSpacePosition;
    float4 ViewSpacePosition;
    float4 WorldSpaceDirection; 
    float4 ViewSpaceDirection; 
    float4 Color;
    
    float SpotAngle;
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;
};

struct LightProperties
{
    uint NumPointLights;
    uint NumSpotLights;
};

struct LightResult
{
    float4 Diffuse;
    float4 Specular;
};

ConstantBuffer<Material> MaterialCB               : register(b0, space1);
ConstantBuffer<LightProperties> LightPropertiesCB : register(b1);

StructuredBuffer<PointLight> PointLights : register(t0);
StructuredBuffer<SpotLight> SpotLights   : register(t1);
Texture2D DiffuseTexture                 : register(t2);

SamplerState LinearRepeatSampler : register(s0);

float3 LinearToSRGB(float3 x)
{
     //Cheaper but less accurate
     //return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
    
    // This is the sRGB curve
    return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
}

float DoDiffuse(float3 N, float3 L)
{
    return max(0, dot(N, L));
}

float DoSpecular(float3 V, float3 N, float3 L)
{
    float3 R = normalize(reflect(-L, N));
    float RdotV = max(0, dot(R, V));

    return pow(RdotV, MaterialCB.SpecularPower);
}

float DoAttenuation(float c, float l, float q, float d)
{
    return 1.0f / (c + l * d + q * d * d);
}

float DoSpotCone(float3 spotDir, float3 L, float spotAngle)
{
    float minCos = cos(spotAngle);
    float maxCos = (minCos + 1.0f) / 2.0f;
    float cosAngle = dot(spotDir, -L);
    return smoothstep(minCos, maxCos, cosAngle);
}

LightResult DoPointLight(PointLight light, float3 V, float3 P, float3 N)
{
    LightResult result;
    float3 L = (light.ViewSpacePosition.xyz - P);
    float d = length(L);
    L = L / d;

    float attenuation = DoAttenuation(light.ConstantAttenuation,
                                      light.LinearAttenuation,
                                      light.QuadraticAttenuation,
                                      d);

    result.Diffuse  = DoDiffuse(N, L) * attenuation * light.Color;
    result.Specular = DoSpecular(V, N, L) * attenuation * light.Color;

    return result;
}

LightResult DoSpotLight(SpotLight light, float3 V, float3 P, float3 N)
{
    LightResult result;
    float3 L = (light.ViewSpacePosition.xyz - P);
    float d = length(L);
    L = L / d;

    float attenuation = DoAttenuation(light.ConstantAttenuation,
                                      light.LinearAttenuation,
                                      light.QuadraticAttenuation,
                                      d);

    float spotIntensity = DoSpotCone(light.ViewSpaceDirection.xyz, L, light.SpotAngle);

    result.Diffuse  = DoDiffuse(N, L) * attenuation * spotIntensity * light.Color;
    result.Specular = DoSpecular(V, N, L) * attenuation * spotIntensity * light.Color;

    return result;
}

LightResult DoLighting(float3 P, float3 N)
{
    uint i;
    
    float3 V = normalize(-P);

    LightResult totalResult = (LightResult) 0;

    for (i = 0; i < LightPropertiesCB.NumPointLights; ++i)
    {
        LightResult result = DoPointLight(PointLights[i], V, P, N);

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
    }

    for (i = 0; i < LightPropertiesCB.NumSpotLights; ++i)
    {
        LightResult result = DoSpotLight(SpotLights[i], V, P, N);

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
    }

    totalResult.Diffuse = saturate(totalResult.Diffuse);
    totalResult.Specular = saturate(totalResult.Specular);

    return totalResult;
}

float4 main(PixelShaderInput IN) : SV_Target
{
    LightResult lit = DoLighting(IN.ViewSpacePosition.xyz, normalize(IN.ViewSpaceNormal));

    float4 emissive = MaterialCB.Emissive;
    float4 ambient  = MaterialCB.Ambient;
    float4 diffuse  = MaterialCB.Diffuse * lit.Diffuse;
    float4 specular = MaterialCB.Specular * lit.Specular;
    float4 texColor = DiffuseTexture.Sample(LinearRepeatSampler, IN.TexCoord);
    
    return (emissive + ambient + diffuse + specular) * texColor;
}