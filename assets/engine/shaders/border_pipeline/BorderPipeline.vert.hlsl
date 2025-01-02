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

    [[vk::location(10)]] float leftWidth;
    [[vk::location(11)]] float topWidth;
    [[vk::location(12)]] float rightWidth;
    [[vk::location(13)]] float bottomWidth;
};

struct Output
{
    float4 position : SV_POSITION;

    [[vk::location(0)]] float2 screenPos : POSITION0;
    [[vk::location(1)]] float2 color : COLOR0;

    [[vk::location(2)]] float2 topLeftPos;
    [[vk::location(3)]] float2 topRightPos;
    [[vk::location(4)]] float2 bottomLeftPos;
    [[vk::location(5)]] float2 bottomRightPos;

    [[vk::location(6)]] float2 topLeftInnerPos;
    [[vk::location(7)]] float2 bottomLeftInnerPos;
    [[vk::location(8)]] float2 topRightInnerPos;
    [[vk::location(9)]] float2 bottomRightInnerPos;

    [[vk::location(10)]] float topLeftRadius;
    [[vk::location(11)]] float bottomLeftRadius;
    [[vk::location(12)]] float topRightRadius;
    [[vk::location(13)]] float bottomRightRadius;

    [[vk::location(14)]] float topLeftWidth;
    [[vk::location(15)]] float topRightWidth;
    [[vk::location(16)]] float bottomLeftWidth;
    [[vk::location(17)]] float bottomRightWidth;
    [[vk::location(18)]] float leftWidth;
    [[vk::location(19)]] float topWidth;
    [[vk::location(20)]] float rightWidth;
    [[vk::location(21)]] float bottomWidth;
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
    
    output.position = pushConsts.model * float4(input.position.x, input.position.y, 0.0, 1.0);
    output.color = input.color;
    output.screenPos = input.position.xy;

    output.topLeftPos = input.topLeftPos;
    output.topRightPos = input.topRightPos;
    output.bottomLeftPos = input.bottomLeftPos;
    output.bottomRightPos = input.bottomRightPos;

    output.topLeftInnerPos = input.topLeftPos + float2(input.topLeftRadius.x, input.topLeftRadius.y);
    output.bottomLeftInnerPos = input.bottomLeftPos + float2(input.bottomLeftRadius.x, -input.bottomLeftRadius.y);
    output.topRightInnerPos = input.topRightPos + float2(-input.topRightRadius.x, input.topRightRadius.y);
    output.bottomRightInnerPos = input.bottomRightPos + float2(-input.bottomRightRadius.x, -input.bottomRightRadius.y);
    
    output.topLeftRadius = Radius(input.topLeftRadius);
    output.bottomLeftRadius = Radius(input.bottomLeftRadius);
    output.topRightRadius = Radius(input.topRightRadius);
    output.bottomRightRadius = Radius(input.bottomRightRadius);

    output.leftWidth = input.leftWidth;
    output.topWidth = input.topWidth;
    output.rightWidth = input.rightWidth;
    output.bottomWidth = input.bottomWidth;
    output.topLeftWidth = lerp(input.leftWidth, input.topWidth, 0.5);
    output.topRightWidth = lerp(input.topWidth, input.rightWidth, 0.5);
    output.bottomLeftWidth = lerp(input.bottomWidth, input.leftWidth, 0.5);
    output.bottomRightWidth = lerp(input.bottomWidth, input.rightWidth, 0.5);    

    return output;
}