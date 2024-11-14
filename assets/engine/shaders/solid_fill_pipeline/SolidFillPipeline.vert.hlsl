struct Input
{
    // Per vertex
    [[vk::location(0)]] float2 position : POSITION0;
    [[vk::location(1)]] float4 color : COLOR;
    // Per instance
    [[vk::location(2)]] float2 topLeftPos;
    [[vk::location(3)]] float topLeftRadius;

    [[vk::location(4)]] float2 bottomLeftPos;
    [[vk::location(5)]] float bottomLeftRadius;

    [[vk::location(6)]] float2 topRightPos;
    [[vk::location(7)]] float topRightRadius;

    [[vk::location(8)]] float2 bottomRightPos;
    [[vk::location(9)]] float bottomRightRadius;
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
}

Output main(Input input)
{
    Output output;

    float4 position = float4(input.position, 0.0, 1.0);

    output.position = position;
    output.screenPos = position.xy;
    output.color = input.color;

    float2 center = (input.topLeftPos + input.bottomLeftPos + input.topRightPos + input.bottomRightPos) * 0.25;
    float2 maxDiff = input.bottomRightPos - center;

    float topLeftRadiusX = min(maxDiff.x, input.topLeftRadius);
    float topLeftRadiusY = min(maxDiff.y, input.topLeftRadius);
    float bottomLeftRadiusX = min(maxDiff.x, input.bottomLeftRadius);
    float bottomLeftRadiusY = min(maxDiff.y, input.bottomLeftRadius);
    float topRightRadiusX = min(maxDiff.x, input.topRightRadius);
    float topRightRadiusY = min(maxDiff.y, input.topRightRadius);
    float bottomRightRadiusX = min(maxDiff.x, input.bottomRightRadius);
    float bottomRightRadiusY = min(maxDiff.y, input.bottomRightRadius);

    // This is probably why they have separated radius into x and y component
    output.topLeftInnerPos = input.topLeftPos + float2(topLeftRadiusX, topLeftRadiusY);
    output.bottomLeftInnerPos = input.bottomLeftPos + float2(bottomLeftRadiusX, -bottomLeftRadiusY);
    output.topRightInnerPos = input.topRightPos + float2(-topRightRadiusX, topRightRadiusY);
    output.bottomRightInnerPos = input.bottomRightPos + float2(-bottomRightRadiusX, -bottomRightRadiusY);

    output.topLeftRadius = min(topLeftRadiusX, topLeftRadiusY);
    output.bottomLeftRadius = min(bottomLeftRadiusX, bottomLeftRadiusY);
    output.topRightRadius = min(topRightRadiusX, topRightRadiusY);
    output.bottomRightRadius = min(bottomRightRadiusX, bottomRightRadiusY);

    return output;
}