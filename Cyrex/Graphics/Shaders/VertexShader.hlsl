struct Mat
{
    matrix ModelMatrix;
    matrix ModelViewMatrix;
    matrix InverseTransposeModelViewMatrix;
    matrix ModelViewProjectionMatrix;
};

ConstantBuffer<Mat> MatCB : register(b0);

struct VertexPositionNormalTexture
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 ViewSpacePosition : POSITION;
    float3 ViewSpaceNormal   : NORMAL;
    float2 TexCoord          : TEXCOORD;
    float4 Position          : SV_Position;
};

VertexShaderOutput main(VertexPositionNormalTexture IN)
{
    VertexShaderOutput OUT;

    OUT.Position          = mul(MatCB.ModelViewProjectionMatrix, float4(IN.Position, 1.0f));
    OUT.ViewSpacePosition = mul(MatCB.ModelViewMatrix, float4(IN.Position, 1.0f));
    OUT.ViewSpaceNormal   = mul((float3x3) MatCB.InverseTransposeModelViewMatrix, IN.Normal);
    OUT.TexCoord          = IN.TexCoord;

    return OUT;
}