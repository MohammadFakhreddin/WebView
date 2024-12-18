struct Input
{
    [[vk::location(0)]] float2 position : POSITION0;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
    [[vk::location(2)]] float3 color : COLOR;
};

struct Output
{
    float4 position : SV_POSITION;
    [[vk::location(0)]] float2 uv : TEXCOORD0;
    [[vk::location(1)]] float3 color : COLOR;
};

struct PushConsts
{    
    float4x4 model;
};
[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};


Output main(Input input)
{
    Output output;
    output.position = pushConsts.model * float4(input.position.x, input.position.y, 0.0, 1.0);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}