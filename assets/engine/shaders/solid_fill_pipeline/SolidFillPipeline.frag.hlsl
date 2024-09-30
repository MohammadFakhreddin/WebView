struct Input
{
    [[vk::location(0)]] float3 screenPos : POSITION0;
    [[vk::location(1)]] float3 color : COLOR;

    [[vk::location(2)]] float3 innerPos0;
    [[vk::location(3)]] float3 innerPos1;
    [[vk::location(4)]] float3 innerPos2;
    [[vk::location(5)]] float3 innerPos3;
    [[vk::location(6)]] float borderRadius;
};

struct Output
{
    float4 color : SV_Target0;
};

float DistanceFromLine(float3 screenPos, float3 linePos0, float3 linePos1)
{
    float3 screenVec = screenPos - linePos0;
    float3 lineVec = normalize(linePos1 - linePos0);
    float dist = screenVec - dot(screenVec, lineVec);
    return abs(dist);
}

float4 main(Input input) : SV_TARGET
{
    float dist0 = DistanceFromLine(input.screenPos, input.pos0, input.pos1);
    float dist1 = DistanceFromLine(input.screenPos, input.pos1, input.pos2);
    float dist2 = DistanceFromLine(input.screenPos, input.pos2, input.pos3);
    float dist3 = DistanceFromLine(input.screenPos, input.pos3, input.pos0);

    float radius = input.radius;
    if (dist0 > radius && dist1 > radius && dist2 > radius && dist3 > radius)
    {
        discard;
    }

    return float4(input.color, 1.0);
}