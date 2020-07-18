#ifndef __DEFAULT_HLSL__
#define __DEFAULT_HLSL__

#ifndef __TYPES_HLSL__
#include "types.hlsl"
#endif // __TYPES_HLSL__

struct vs_input_t
{
   // we are not defining our own input data; 
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t
{
    float4 position : SV_POSITION;

    float4 color : COLOR;
    float2 uv : UV;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t vert_main( vs_input_t input )
{
    v2f_t v2f = (v2f_t) 0;

    // forward vertex input onto the next stage
    float4 worldPos = float4(input.position, 1.0f);

    float4 objectPos = mul( OBJECT_MODEL, worldPos );
    float4 cameraPos = mul( WORLD_TO_CAMERA, objectPos );
    float4 clipPos = mul( CAMERA_TO_CLIP_TRANSFORM, cameraPos );

    v2f.position = clipPos;
    v2f.color = input.color * OBJECT_TINT;
    v2f.uv = input.uv;

    return v2f;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 frag_main( v2f_t input ): SV_Target0
{
    float4 color = tDiffuse.Sample( sSampler, input.uv );
    return color * input.color;
}

#endif // __DEFAULT_HLSL__