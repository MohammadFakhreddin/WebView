struct Input
{
    // Per vertex
    [[vk::location(0)]] float2 position : POSITION0;
    [[vk::location(1)]] float3 color : COLOR;
    // Per instance
    [[vk::location(2)]] float2 innerPos0;
    [[vk::location(3)]] float2 innerPos1;
    [[vk::location(4)]] float2 innerPos2;
    [[vk::location(5)]] float2 innerPos3;
    
    [[vk::location(6)]] float borderRadius;
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
    output.color = input.color;

    // float2 center = (input.innerPos0 + input.innerPos1 + input.innerPos2 + input.innerPos3) * 0.25;
    
    // float halfWidth = abs(input.innerPos0.x - center.x) - input.borderRadius;
    // float halfHeight = abs(input.innerPos0.y - center.y) - input.borderRadius;

    // output.innerPos0 = float2(center.x - halfWidth, center.y - halfHeight);
    // output.innerPos1 = float2(center.x - halfWidth, center.y + halfHeight);
    // output.innerPos2 = float2(center.x + halfWidth, center.y - halfHeight);
    // output.innerPos3 = float2(center.x + halfWidth, center.y + halfHeight);
    // output.borderRadius = input.borderRadius;
    
    output.innerPos0 = input.innerPos0;
    output.innerPos1 = input.innerPos1;
    output.innerPos2 = input.innerPos2;
    output.innerPos3 = input.innerPos3;
    
    output.borderRadius = input.borderRadius;
    
    return output;
}