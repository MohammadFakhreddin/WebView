struct Input
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
};

struct Output
{
    float4 color : SV_Target0;
};

Texture2D imageTexture : register(t0, space0);
SamplerState imageSampler : register(s0, space0);

float4 main(Input input) : SV_TARGET
{
    return imageTexture.Sample(imageSampler, input.uv);
}