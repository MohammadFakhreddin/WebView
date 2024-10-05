struct Input
{
    // Per vertex
    [[vk::location(0)]] float2 position : POSITION0;
    [[vk::location(1)]] float3 color : COLOR;
    // Per instance
    [[vk::location(2)]] float2 innerPos0;
    [[vk::location(3)]] float3 color0;
    [[vk::location(4)]] float2 innerPos1;
    [[vk::location(5)]] float3 color1;
    [[vk::location(6)]] float2 innerPos2;
    [[vk::location(7)]] float3 color2;
    [[vk::location(8)]] float2 innerPos3;
    [[vk::location(9)]] float3 color3;
    
    [[vk::location(10)]] float borderRadius;
};

struct Output
{
    float4 position : SV_POSITION;

    [[vk::location(0)]] float3 screenPos : POSITION0;
    [[vk::location(1)]] float3 color : COLOR;

    [[vk::location(2)]] float2 innerPos0;
    [[vk::location(3)]] float2 innerPos1;
    [[vk::location(4)]] float2 innerPos2;
    [[vk::location(5)]] float2 innerPos3;
    [[vk::location(6)]] float borderRadius;
}

Output main(Input input)
{
    Output output;

    float4 position = float4(input.position, 0.0, 1.0);

    output.position = position;
    output.screenPos = position.xyz;
    output.color = input.color + 0.0001 * (input.color0 + input.color1 + input.color2 + input.color3);
    
    output.innerPos0 = input.innerPos0;
    output.innerPos1 = input.innerPos1;
    output.innerPos2 = input.innerPos2;
    output.innerPos3 = input.innerPos3;
    output.borderRadius = input.borderRadius;
    
    return output;
}