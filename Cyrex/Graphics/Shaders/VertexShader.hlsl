struct Mat
{
    matrix ModelMatrix;
    matrix ModelViewMatrix;
    matrix InverseTransposeModelViewMatrix;
    matrix ModelViewProjectionMatrix;
};

ConstantBuffer<Mat> MatCB : register(b0);

struct VertexPositionNormalTangentBitangentTexture
{
    float3 Position  : POSITION;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 TexCoord  : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 ViewSpacePosition  : POSITION;
    float3 ViewSpaceNormal    : NORMAL;
    float3 ViewSpaceTangent   : TANGENT;
    float3 ViewSpaceBitangent : BITANGENT;
    float2 TexCoord           : TEXCOORD;
    float4 Position           : SV_Position;
};

VertexShaderOutput main(VertexPositionNormalTangentBitangentTexture IN)
{
    VertexShaderOutput OUT;

    OUT.ViewSpacePosition  = mul(MatCB.ModelViewMatrix, float4(IN.Position, 1.0f));
    OUT.ViewSpaceNormal    = mul((float3x3)MatCB.InverseTransposeModelViewMatrix, IN.Normal);
    OUT.ViewSpaceTangent   = mul((float3x3)MatCB.InverseTransposeModelViewMatrix, IN.Tangent);
    OUT.ViewSpaceBitangent = mul((float3x3)MatCB.InverseTransposeModelViewMatrix, IN.Bitangent);
    OUT.TexCoord           = IN.TexCoord.xy;
    OUT.Position           = mul(MatCB.ModelViewProjectionMatrix, float4(IN.Position, 1.0f));

    return OUT;
}