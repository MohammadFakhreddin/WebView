struct Input
{
    // Per vertex
    [[vk::location(0)]] float2 position : POSITION0;
    [[vk::location(1)]] float4 color : COLOR;
    // Per instance
    [[vk::location(2)]] float2 topLeftPos;
    [[vk::location(3)]] float2 topLeftRadius;

    [[vk::location(4)]] float2 bottomLeftPos;
    [[vk::location(5)]] float2 bottomLeftRadius;

    [[vk::location(6)]] float2 topRightPos;
    [[vk::location(7)]] float2 topRightRadius;

    [[vk::location(8)]] float2 bottomRightPos;
    [[vk::location(9)]] float2 bottomRightRadius;
};

struct Output
{
    float4 position : SV_POSITION;

    [[vk::location(0)]] float2 screenPos : POSITION0;
    [[vk::location(1)]] float4 color : COLOR;

    [[vk::location(2)]] float2 topLeftInnerPos;
    [[vk::location(3)]] float2 bottomLeftInnerPos;
    [[vk::location(4)]] float2 topRightInnerPos;
    [[vk::location(5)]] float2 bottomRightInnerPos;

    [[vk::location(6)]] float topLeftRadius;
    [[vk::location(7)]] float bottomLeftRadius;
    [[vk::location(8)]] float topRightRadius;
    [[vk::location(9)]] float bottomRightRadius;
};

struct PushConsts
{    
    float4x4 model;
};
[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

float Radius(float2 xy)
{
    return xy.y;
};

Output main(Input input)
{
    Output output;

    float4 position = pushConsts.model * float4(input.position.x, input.position.y, 0.0, 1.0);

    output.position = position;
    output.screenPos = input.position.xy;
    output.color = input.color;

    // This is probably why they have separated radius into x and y component
    output.topLeftInnerPos = input.topLeftPos + float2(input.topLeftRadius.x, input.topLeftRadius.y);
    output.bottomLeftInnerPos = input.bottomLeftPos + float2(input.bottomLeftRadius.x, -input.bottomLeftRadius.y);
    output.topRightInnerPos = input.topRightPos + float2(-input.topRightRadius.x, input.topRightRadius.y);
    output.bottomRightInnerPos = input.bottomRightPos + float2(-input.bottomRightRadius.x, -input.bottomRightRadius.y);

    output.topLeftRadius = Radius(input.topLeftRadius);
    output.bottomLeftRadius = Radius(input.bottomLeftRadius);
    output.topRightRadius = Radius(input.topRightRadius);
    output.bottomRightRadius = Radius(input.bottomRightRadius);

    return output;
}