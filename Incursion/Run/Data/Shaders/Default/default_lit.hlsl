#ifndef __DEFAULT_LIT_HLSL__
#define __DEFAULT_LIT_HLSL__

#ifndef __TYPES_HLSL__
#include "types.hlsl"
#endif // __TYPES_HLSL__

#ifndef __LIGHTING_UTILS_HLSL__
#include "lighting_utils.hlsl"
#endif // __LIGHTING_UTILS_HLSL__

struct vs_input_t
{
   // we are not defining our own input data; 
    float3 position : POSITION;
    float4 color : COLOR;
    
    float2 uv : TEXCOORD;

    float3 tangent: TANGENT;
    float3 bitangent: BITANGENT;
    float3 normal : NORMAL;
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
    
    float3 worldPosition : WORLD_POSITION;
    float3 worldTangent: world_TANGENT;
    float3 worldBitangent: WORLD_BITANGENT;
    float3 worldNormal: WORLD_NOMRAL;

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

    float4 worldTangent = mul( OBJECT_MODEL, float4(input.tangent, 0.f) );
    float4 worldBitangent = mul( OBJECT_MODEL, float4(input.bitangent, 0.f) );
    float4 worldNormal = mul( OBJECT_MODEL, float4(input.normal, 0.f) );

    v2f.position = clipPos;
    v2f.color = input.color * OBJECT_TINT;
    v2f.uv = input.uv;
    v2f.worldPosition = objectPos.xyz;

    v2f.worldTangent = worldTangent.xyz;
    v2f.worldBitangent = worldBitangent.xyz;
    v2f.worldNormal = worldNormal.xyz;

    return v2f;
}

struct fo_t
{
    float4 color: SV_Target0;
    float4 bloom: SV_Target1;
//    float4 normal: SV_Target2;
//    float4 albeto: SV_Target3;
//    float4 tangent: SV_Target4;
};

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
fo_t frag_main( v2f_t input )
{
    float4 textureColor = tDiffuse.Sample( sSampler, input.uv );
    float3 textureNormal = tNormal.Sample( sSampler, input.uv ).xyz;
    textureNormal = normalize(float3( (textureNormal.x * 2) - 1, (textureNormal.y * 2) - 1, textureNormal.z ));
    float3x3 TBN = float3x3( normalize( input.worldTangent ), normalize( input.worldBitangent ), normalize( input.worldNormal ) );

    float3 normal = normalize( mul( textureNormal, TBN ) );

    float4 finalColor = calculate_lighting_at_point( input.worldPosition, normal, textureColor, input.color );
    float3 bloom = max( float3(0, 0, 0), finalColor - float3(1, 1, 1) );

    finalColor = calculate_fog( input.worldPosition, finalColor );


    fo_t output = (fo_t) 0;
    output.color = finalColor;
    output.bloom = float4( bloom, 1);
//    output.normal = float4((normal + float3(1, 1, 1)) * .5f, 1);
//    output.tangent = float4((input.worldTangent + float3(1, 1, 1) * .5f), 1);
//    output.albeto = textureColor;

    return output;
}

#endif // __DEFAULT_LIT_HLSL__