#ifndef __TYPES_HLSL__
#define __TYPES_HLSL__

static const uint MAX_LIGHTS = 8;

cbuffer frame_constants : register(b0)
{
    float SYSTEM_TIME_SECONDS;
    float SYSTEM_TIME_DELTA_SECONDS;
    float GAME_TIME_SECONDS;
    float GAME_TIME_DELTA_SECONDS;

    float GAMMA;
    float INVERSE_GAMMA;
};

cbuffer camera_constants : register(b1)
{
    float4x4 CAMERA_TO_CLIP_TRANSFORM;
    float4x4 WORLD_TO_CAMERA;

    float3 CAMERA_POSITION;
};

cbuffer object_constants : register(b2)
{
    float4x4 OBJECT_MODEL;
    float4 OBJECT_TINT;

    float SPECULAR_FACTOR;
    float SPECULAR_POWER;
};

struct light_t
{
    float3 worldPosition;

    float4 color;

    float3 attenuation;
    float cosHalfInner;

    float3 specularAttenuation;
    float cosHalfOuter;

    float3 direction;
    float isDirection;
};

cbuffer light_constants: register(b3)
{
    float4 AMBIENT;
    light_t LIGHTS[ MAX_LIGHTS ];
};

cbuffer fog_constants: register(b4)
{
    float4 NEAR_FOG_COLOR;
    float4 FAR_FOG_COLOR;

    float NEAR_FOG_DISTANCE;
    float FAR_FOG_DISTANCE;
};

Texture2D <float4> tDiffuse : register(t0);
Texture2D <float4> tNormal : register(t1);

SamplerState sSampler : register(s0);

#endif // __TYPES_HLSL__