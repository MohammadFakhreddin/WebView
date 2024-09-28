Texture2D textureFont : register(t0, space0);
SamplerState samplerFont : register(s0, space0);

struct Input
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
    [[vk::location(1)]] float3 color : COLOR;
};

struct Output
{
    float4 color : SV_Target0;
};

float4 main(Input input) : SV_TARGET
{
    float strength = textureFont.Sample(samplerFont, input.uv).r;
    if (strength < 0.5)
    {
        discard;
    }
    return float4(input.color * strength, 1.0);
}