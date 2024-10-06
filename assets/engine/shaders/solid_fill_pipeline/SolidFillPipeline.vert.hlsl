struct Input
{
    // Per vertex
    [[vk::location(0)]] float2 position : POSITION0;
    [[vk::location(1)]] float3 color : COLOR;
    // Per instance
    [[vk::location(2)]] float2 position0;
    [[vk::location(3)]] float2 position1;
    [[vk::location(4)]] float2 position2;
    [[vk::location(5)]] float2 position3;
    
    [[vk::location(6)]] float borderRadius;
};

struct Output
{
    float4 position : SV_POSITION;

    [[vk::location(0)]] float2 screenPos : POSITION0;
    [[vk::location(1)]] float3 color : COLOR;

    [[vk::location(2)]] float2 topLeft;
    [[vk::location(3)]] float2 bottomLeft;
    [[vk::location(4)]] float2 topRight;
    [[vk::location(5)]] float2 bottomRight;

    [[vk::location(6)]] float borderRadius;
}

Output main(Input input)
{
    Output output;

    float4 position = float4(input.position, 0.0, 1.0);

    output.position = position;
    output.screenPos = position.xy;
    output.color = input.color;

    float2 center = (input.position0 + input.position1 + input.position2 + input.position3) * 0.25;
    
    float halfWidth = abs(input.position0.x - center.x) - input.borderRadius;
    float halfHeight = abs(input.position0.y - center.y) - input.borderRadius;

    output.topLeft = float2(center.x - halfWidth, center.y - halfHeight);
    output.bottomLeft = float2(center.x - halfWidth, center.y + halfHeight);
    output.topRight = float2(center.x + halfWidth, center.y - halfHeight);
    output.bottomRight = float2(center.x + halfWidth, center.y + halfHeight);

    output.borderRadius = input.borderRadius;
    
    return output;
}