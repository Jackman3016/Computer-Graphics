struct VSOut
{
    float4 pos : SV_Position;
    float3 color : COLOR;
};

cbuffer CBuf
{
    row_major matrix transform;//对于二维数组，GPU按列存储，CPU按行存储，除了转置，可以使用关键字告诉GPU按行读取，但GPU按行计算会变慢
};

VSOut main(float2 pos : POSITION, float3 color : COLOR) : SV_POSITION
{
    VSOut vso;
    vso.pos = mul(float4(pos.x, pos.y, 0.0f, 1.0f), transform);//使用cbuffer不用创建struct
    vso.color = color;
    return VSOut;
}
