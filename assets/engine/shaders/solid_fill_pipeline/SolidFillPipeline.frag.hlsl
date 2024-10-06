struct Input
{
    [[vk::location(0)]] float2 screenPos : POSITION0;
    [[vk::location(1)]] float3 color : COLOR;

    [[vk::location(2)]] float2 topLeft;
    [[vk::location(3)]] float2 bottomLeft;
    [[vk::location(4)]] float2 topRight;
    [[vk::location(5)]] float2 bottomRight;

    [[vk::location(6)]] float borderRadius;
};

struct Output
{
    float4 color : SV_Target0;
};

float Distance(float2 pos0, float2 pos1)
{
    return length(pos0 - pos1);
}

float4 main(Input input) : SV_TARGET
{
    if (input.screenPos.x < input.topLeft.x)
    {
        if (input.screenPos.y < input.topLeft.y)
        {
            if (Distance(input.screenPos, input.topLeft) > input.borderRadius)
            {
                discard;
            }
        }
        else if (input.screenPos.y > input.bottomLeft.y)
        {
            if (Distance(input.screenPos, input.bottomLeft) > input.borderRadius)
            {
                discard;
            }
        }
    }
    if (input.screenPos.x > input.topRight.x)
    {
        if (input.screenPos.y < input.topRight.y)
        {
            if (Distance(input.screenPos, input.topRight) > input.borderRadius)
            {
                discard;
            }
        }
        else if (input.screenPos.y > input.bottomRight.y)
        {
            if (Distance(input.screenPos, input.bottomRight) > input.borderRadius)
            {
                discard;
            }
        }
    }

    return float4(input.color, 1.0);
}