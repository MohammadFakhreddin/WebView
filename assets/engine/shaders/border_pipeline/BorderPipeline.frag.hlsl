struct Input
{
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
    bool alreadyChecked = false;
    if (input.screenPos.x < input.topLeftInnerPos.x)
    {
        if (input.screenPos.y < input.topLeftInnerPos.y)
        {
            float distance = Distance(input.screenPos, input.topLeftInnerPos);
            if (distance > input.topLeftRadius || distance < input.topLeftRadius - input.topLeftWidth)
            {
                discard;
            }
            alreadyChecked = true;
        }
        else if (input.screenPos.y > input.bottomLeftInnerPos.y)
        {
            float distance = Distance(input.screenPos, input.bottomLeftInnerPos);
            if (distance > input.bottomLeftRadius || distance < input.bottomLeftRadius - input.bottomLeftWidth)
            {
                discard;
            }
            alreadyChecked = true;
        }
    }
    if (input.screenPos.x > input.topRightInnerPos.x)
    {
        if (input.screenPos.y < input.topRightInnerPos.y)
        {
            float distance = Distance(input.screenPos, input.topRightInnerPos);
            if (distance > input.topRightRadius || distance < input.topRightRadius - input.topRightWidth)
            {
                discard;
            }
            alreadyChecked = true;
        }
        else if (input.screenPos.y > input.bottomRightInnerPos.y)
        {
            float distance = Distance(input.screenPos, input.bottomRightInnerPos); 
            if (distance > input.bottomRightRadius || distance < input.bottomRightRadius - input.bottomRightWidth)
            {
                discard;
            }
            alreadyChecked = true;
        }
    }

    if (alreadyChecked == false)
    {
        float2 topRightDist = abs(input.screenPos - input.topRightPos);
        float2 topLeftDist = abs(input.screenPos - input.topLeftPos);
        float2 bottomRightDist = abs(input.screenPos - input.bottomRightPos);
        float2 bottomLeftDist = abs(input.screenPos - input.bottomLeftPos);

        if (topRightDist.x > input.rightWidth && 
            topRightDist.y > input.topWidth && 
            topLeftDist.x > input.leftWidth && 
            topLeftDist.y > input.topWidth && 
            bottomRightDist.x > input.rightWidth && 
            bottomRightDist.y > input.bottomWidth && 
            bottomLeftDist.x > input.leftWidth && 
            bottomLeftDist.y > input.bottomWidth)
        {
            discard;
        }
    }

    return float4(input.color, 0, 1.0f);
}