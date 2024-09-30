#include "../ColorUtils.hlsl"

struct PSIn {
    float4 position : SV_POSITION;
};

struct PSOut {
    float4 color : SV_Target0;
};

struct PushConsts
{    
    float4x4 model;
    float4 color;
    float pointSize;
    float placeholder0;
    float placeholder1;
    float placeholder2;
};

[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

PSOut main(PSIn input) {
    PSOut output;

    float3 color = pushConsts.color.rgb;
    // exposure tone mapping
    color = ApplyExposureToneMapping(color);
    // Gamma correct
    color = ApplyGammaCorrection(color); 

    output.color = float4(color, pushConsts.color.a);
    return output;
}