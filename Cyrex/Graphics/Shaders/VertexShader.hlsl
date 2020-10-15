struct ModelViewProjection {
	matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexposColor {
	float3 Position : POSITION;
	float3 Color    : COLOR;
};

struct VertexShaderoutput {
	float4 Color    : COLOR;
	float4 Position : SV_POSITION;
};

VertexShaderOutput main(VertexPosColor IN) : SV_POSITION
{
	VertexShaderOutput OUT;

    OUT.Position = mul(ModelViewProjection.MVP, float4(IN.POSITION, 1.0f));
	OUT.Color = float4(IN.Color, 1.0f);
	return OUT;
}