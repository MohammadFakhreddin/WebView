struct Input
{
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
    float a = input.color.a;
    if (input.screenPos.x < input.topLeftInnerPos.x)
    {
        if (input.screenPos.y < input.topLeftInnerPos.y)
        {
            if (Distance(input.screenPos, input.topLeftInnerPos) > input.topLeftRadius)
            {
                discard;
            }
        }
        else if (input.screenPos.y > input.bottomLeftInnerPos.y)
        {
            if (Distance(input.screenPos, input.bottomLeftInnerPos) > input.bottomLeftRadius)
            {
                discard;
            }
        }
    }
    if (input.screenPos.x > input.topRightInnerPos.x)
    {
        if (input.screenPos.y < input.topRightInnerPos.y)
        {
            if (Distance(input.screenPos, input.topRightInnerPos) > input.topRightRadius)
            {
                discard;
            }
        }
        else if (input.screenPos.y > input.bottomRightInnerPos.y)
        {
            if (Distance(input.screenPos, input.bottomRightInnerPos) > input.bottomRightRadius)
            {
                discard;
            }
        }
    }

    return input.color;
}