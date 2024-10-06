struct Input
{
    [[vk::location(0)]] float3 screenPos : POSITION0;
    [[vk::location(1)]] float3 color : COLOR;

    [[vk::location(2)]] float2 innerPos0;
    [[vk::location(3)]] float2 innerPos1;
    [[vk::location(4)]] float2 innerPos2;
    [[vk::location(5)]] float2 innerPos3;
    [[vk::location(6)]] float borderRadius;
};

struct Output
{
    float4 color : SV_Target0;
};

float DistanceFromLine(float2 screenPos, float2 linePos0, float2 linePos1)
{
    float2 screenVec = screenPos - linePos0;
    float2 lineVec = normalize(linePos1 - linePos0);
    float dist = length(screenVec) - dot(screenVec, lineVec);
    return abs(dist);
}

float4 main(Input input) : SV_TARGET
{
    // float dist0 = DistanceFromLine(input.screenPos, input.innerPos0, input.innerPos1);
    // float dist1 = DistanceFromLine(input.screenPos, input.innerPos1, input.innerPos2);
    // float dist2 = DistanceFromLine(input.screenPos, input.innerPos2, input.innerPos3);
    // float dist3 = DistanceFromLine(input.screenPos, input.innerPos3, input.innerPos0);

    // float radius = input.borderRadius;
    // if (dist0 > radius && dist1 > radius && dist2 > radius && dist3 > radius)
    // {
    //     discard;
    // }

    return float4(input.color, 1.0);
}