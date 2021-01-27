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

ConstantBuffer<Material> MaterialCB : register(b0, space1);
Texture2D DiffuseTexture            : register(t2);

SamplerState LinearRepeatSampler    : register(s0);

float3 LinearToSRGB(float3 x)
{
    //Cheaper but less accurate
    //return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
    
    // This is the sRGB curve
    return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
}

float4 main(PixelShaderInput IN) : SV_Target
{
    float4 emissive = MaterialCB.Emissive;
    float4 ambient  = MaterialCB.Ambient;
    float4 diffuse  = MaterialCB.Diffuse;
    float4 specular = MaterialCB.Specular;
    float4 texColor = DiffuseTexture.Sample(LinearRepeatSampler, IN.TexCoord);

    return (emissive + ambient + diffuse + specular) * texColor * -IN.ViewSpaceNormal.z;
}