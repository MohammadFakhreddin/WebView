struct VSIn {
    float3 position : POSITION0;
};

struct VSOut {
    float4 position : SV_POSITION;
};

struct PushConsts
{
    float4x4 model;
    float4 color : COLOR0;
};

[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

VSOut main(VSIn input) {
    VSOut output;
    output.position = mul(pushConsts.model, float4(input.position, 1.0));
    return output;
}