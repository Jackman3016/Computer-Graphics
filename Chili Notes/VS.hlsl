struct VSOut {
	float4 pos: SV_Position;
	float3 color : COLOR;
};

VSOut main(float2 pos : POSITION, float3 color : COLOR) : SV_POSITION
{
	VSOut vso;
	vso.pos = float4(posx, pos.y, 0.0f, 1.0f);
	vso.color = color;
	return VSOut;
}
