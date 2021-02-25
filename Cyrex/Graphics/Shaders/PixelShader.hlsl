struct PixelShaderInput
{
    float4 ViewSpacePosition  : POSITION;
    float3 ViewSpaceNormal    : NORMAL;
    float3 ViewSpaceTangent   : TANGENT;
    float3 ViewSpaceBitangent : BITANGENT;
    float2 TexCoord           : TEXCOORD;
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
                             
    bool  HasAmbientTexture;
    bool  HasEmissiveTexture;
    bool  HasDiffuseTexture;
    bool  HasSpecularTexture;
    bool  HasSpecularPowerTexture;
    bool  HasNormalTexture;
    bool  HasBumpTexture;
    bool  HasOpacityTexture;
};

#if ENABLE_LIGHTING
struct PointLight
{
    float4 WorldSpacePosition; 
    float4 ViewSpacePosition; 
    float4 Color;

    float  Ambient;
    float  ConstantAttenuation;
    float  LinearAttenuation;
    float  QuadraticAttenuation;
};

struct SpotLight
{
    float4 WorldSpacePosition; 
    float4 ViewSpacePosition;
    float4 WorldSpaceDirection; 
    float4 ViewSpaceDirection; 
    float4 Color;

    float  Ambient;
    float  SpotAngle;
    float  ConstantAttenuation;
    float  LinearAttenuation;
    float  QuadraticAttenuation;
};

struct DirectionalLight
{
    float4 WorldSpaceDirection;
    float4 ViewSpaceDirection;
    float4 Color;

    float Ambient;
    float3 Padding;
};

struct LightProperties
{
    uint NumPointLights;
    uint NumSpotLights;
    uint NumDirectionalLights;
};

struct LightResult
{
    float4 Diffuse;
    float4 Specular;
    float4 Ambient;
};

ConstantBuffer<LightProperties> LightPropertiesCB : register(b1);

StructuredBuffer<PointLight> PointLights             : register(t0);
StructuredBuffer<SpotLight> SpotLights               : register(t1);
StructuredBuffer<DirectionalLight> DirectionalLights : register(t2);
#endif // ENABLE_LIGHTING

ConstantBuffer<Material> MaterialCB : register(b0, space1);

// Textures
Texture2D AmbientTexture       : register(t3);
Texture2D EmissiveTexture      : register(t4);
Texture2D DiffuseTexture       : register(t5);
Texture2D SpecularTexture      : register(t6);
Texture2D SpecularPowerTexture : register(t7);
Texture2D NormalTexture        : register(t8);
Texture2D BumpTexture          : register(t9);
Texture2D OpacityTexture       : register(t10);

SamplerState TextureSampler    : register(s0);

float3 LinearToSRGB(float3 x)
{
    //Cheaper but less accurate
    //return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;

    // This is the sRGB curve
    return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
}

#if ENABLE_LIGHTING
float DoDiffuse(float3 N, float3 L)
{
    return max(0, dot(N, L));
}

float DoSpecular(float3 V, float3 N, float3 L, float specularPower)
{
    float3 R = normalize(reflect(-L, N));
    float RdotV = max(0, dot(R, V));

    return pow(RdotV, specularPower);
}

float DoAttenuation(float c, float l, float q, float d)
{
    return 1.0f / (c + l * d + q * d * d);
}

float DoSpotCone(float3 spotDir, float3 L, float spotAngle)
{
    float minCos   = cos(spotAngle);
    float maxCos   = (minCos + 1.0f) / 2.0f;
    float cosAngle = dot(spotDir, -L);

    return smoothstep(minCos, maxCos, cosAngle);
}

LightResult DoPointLight(PointLight light, float3 V, float3 P, float3 N, float specularPower)
{
    LightResult result;
    float3 L = (light.ViewSpacePosition.xyz - P);
    float d  = length(L);
    L = L / d;

    float attenuation = DoAttenuation(light.ConstantAttenuation,
                                      light.LinearAttenuation,
                                      light.QuadraticAttenuation,
                                      d);

    result.Diffuse  = DoDiffuse(N, L) * attenuation * light.Color;
    result.Specular = DoSpecular(V, N, L, specularPower) * attenuation * light.Color;
    result.Ambient  = light.Color * light.Ambient;

    return result;
}

LightResult DoSpotLight(SpotLight light, float3 V, float3 P, float3 N, float specularPower)
{
    LightResult result;
    float3 L = (light.ViewSpacePosition.xyz - P);
    float d  = length(L);
    L = L / d;

    float attenuation = DoAttenuation(light.ConstantAttenuation,
                                      light.LinearAttenuation,
                                      light.QuadraticAttenuation,
                                      d);

    float spotIntensity = DoSpotCone(light.ViewSpaceDirection.xyz, L, light.SpotAngle);

    result.Diffuse  = DoDiffuse(N, L) * attenuation * spotIntensity * light.Color;
    result.Specular = DoSpecular(V, N, L, specularPower) * attenuation * spotIntensity * light.Color;
    result.Ambient  = light.Color * light.Ambient;

    return result;
}

LightResult DoDirectionalLight(DirectionalLight light, float3 V, float3 P, float3 N, float specularPower)
{
    LightResult result;

    float3 L = normalize(-light.ViewSpaceDirection.xyz);

    result.Diffuse  = light.Color * DoDiffuse(N, L);
    result.Specular = light.Color * DoSpecular(V, N, L, specularPower);
    result.Ambient  = light.Color * light.Ambient;

    return result;
}

LightResult DoLighting(float3 P, float3 N, float specularPower)
{
    uint i;

    // Lighting is performed in view space.
    float3 V = normalize(-P);

    LightResult totalResult = (LightResult)0;

    for (i = 0; i < LightPropertiesCB.NumPointLights; i++)
    {
        LightResult result = DoPointLight(PointLights[i], V, P, N, specularPower);

        totalResult.Diffuse  += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Ambient  += result.Ambient;
    }

    for (i = 0; i < LightPropertiesCB.NumSpotLights; i++)
    {
        LightResult result = DoSpotLight(SpotLights[i], V, P, N, specularPower);

        totalResult.Diffuse  += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Ambient  += result.Ambient;
    }

    for (i = 0; i < LightPropertiesCB.NumDirectionalLights; i++)
    {
        LightResult result = DoDirectionalLight(DirectionalLights[i], V, P, N, specularPower);

        totalResult.Diffuse  += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Ambient  += result.Ambient;
    }

    totalResult.Diffuse  = saturate(totalResult.Diffuse);
    totalResult.Specular = saturate(totalResult.Specular);
    totalResult.Ambient  = saturate(totalResult.Ambient);

    return totalResult;
}
#endif // ENABLE_LIGHTING

float3 ExpandNormal(float3 n)
{
    return n * 2.0f - 1.0f;
}

float3 DoNormalMapping(float3x3 TBN, Texture2D tex, float2 uv)
{
    float3 N = tex.Sample(TextureSampler, uv).xyz;
    N = ExpandNormal(N);

    // Transform normal from tangent space to view space.
    N = mul(N, TBN);
    return normalize(N);
}

float3 DoBumpMapping(float3x3 TBN, Texture2D tex, float2 uv, float bumpScale)
{
    // Sample the heightmap at the current texture coordinate.
    float height_00 = tex.Sample(TextureSampler, uv).r * bumpScale;
    // Sample the heightmap in the U texture coordinate direction.
    float height_10 = tex.Sample(TextureSampler, uv, int2(1, 0)).r * bumpScale;
    // Sample the heightmap in the V texture coordinate direction.
    float height_01 = tex.Sample(TextureSampler, uv, int2(0, 1)).r * bumpScale;

    float3 p_00 = { 0, 0, height_00 };
    float3 p_10 = { 1, 0, height_10 };
    float3 p_01 = { 0, 1, height_01 };

    // normal = tangent x bitangent
    float3 tangent = normalize(p_10 - p_00);
    float3 bitangent = normalize(p_01 - p_00);

    float3 normal = cross(tangent, bitangent);

    // Transform normal from tangent space to view space.
    normal = mul(normal, TBN);

    return normal;
}


// If c is not black, then blend the color with the texture
// otherwise, replace the color with the texture.
float4 SampleTexture(Texture2D t, float2 uv, float4 c)
{
    if (any(c.rgb))
    {
        c *= t.Sample(TextureSampler, uv);
    }
    else
    {
        c = t.Sample(TextureSampler, uv);
    }

    return c;
}

float4 main(PixelShaderInput IN) : SV_Target
{
    Material material = MaterialCB;

// By default, use the alpha component of the diffuse color.
float  alpha = material.Diffuse.a;
if (material.HasOpacityTexture)
{
    alpha = OpacityTexture.Sample(TextureSampler, IN.TexCoord.xy).r;
}

#if ENABLE_DECAL
    if (alpha < 0.1f)
    {
        discard; // Discard the pixel if it is below a certain threshold.
    }
#endif // ENABLE_DECAL

    float4 ambient = material.Ambient;
    float4 emissive = material.Emissive;
    float4 diffuse = material.Diffuse;
    float specularPower = material.SpecularPower;
    float2 uv = IN.TexCoord.xy;

    if (material.HasAmbientTexture)
    {
        ambient = SampleTexture(AmbientTexture, uv, ambient);
    }
    if (material.HasEmissiveTexture)
    {
        emissive = SampleTexture(EmissiveTexture, uv, emissive);
    }
    if (material.HasDiffuseTexture)
    {
        diffuse = SampleTexture(DiffuseTexture, uv, diffuse);
    }
    if (material.HasSpecularPowerTexture)
    {
        specularPower *= SpecularPowerTexture.Sample(TextureSampler, uv).r;
    }

    float3 N;
    // Normal mapping
    if (material.HasNormalTexture)
    {
        float3 tangent   = normalize(IN.ViewSpaceTangent);
        float3 bitangent = normalize(IN.ViewSpaceBitangent);
        float3 normal    = normalize(IN.ViewSpaceNormal);

        float3x3 TBN = float3x3(tangent,
                                 bitangent,
                                 normal);

        N = DoNormalMapping(TBN, NormalTexture, uv);
    }
    else if (material.HasBumpTexture)
    {
        float3 tangent   = normalize(IN.ViewSpaceTangent);
        float3 bitangent = normalize(IN.ViewSpaceBitangent);
        float3 normal    = normalize(IN.ViewSpaceNormal);

        float3x3 TBN = float3x3(tangent,
                                 -bitangent,
                                 normal);

        N = DoBumpMapping(TBN, BumpTexture, uv, material.BumpIntensity);
    }
    else
    {
        N = normalize(IN.ViewSpaceNormal);
    }

    float shadow    = 1;
    float4 specular = 0;

#if ENABLE_LIGHTING
    LightResult lit = DoLighting(IN.ViewSpacePosition.xyz, N, specularPower);
    diffuse *= lit.Diffuse;
    ambient *= lit.Ambient;
   
    if (material.SpecularPower > 1.0f)
    {
        specular = material.Specular;
        if (material.HasSpecularTexture)
        {
            specular = SampleTexture(SpecularTexture, uv, specular);
        }
        specular *= lit.Specular;
    }
#else 
    shadow = -N.z;
#endif // ENABLE_LIGHTING

    return float4((emissive + ambient + diffuse + specular).rgb * shadow, alpha * material.Opacity);
}