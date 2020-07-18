#ifndef __LIGHTING_UTILS_HLSL__
#define __LIGHTING_UTILS_HLSL__

#ifndef __TYPES_HLSL__
#include "types.hlsl"
#endif // __TYPES_HLSL__

float3 normal_from_color( float3 colorInput )
{
    return float3((colorInput.x * 2) - 1, (colorInput.y * 2) - 1, colorInput.z);
}

float2 calculate_light_factors_at_point( float3 worldPosition, float3 worldNormal, light_t light )
{
    float3 toLight = light.worldPosition - worldPosition;
    float toLightDistance = length( toLight );
    float3 toLightDirection = normalize( toLight );

    float3 lightDirection = normalize( lerp( toLightDirection, -light.direction, light.isDirection ) );
    float3 attenuation = float3(1, toLightDistance, toLightDistance * toLightDistance);

    float spotlight = dot( -toLightDirection, light.direction );
    float spotlightAttenuation = smoothstep( light.cosHalfOuter, light.cosHalfInner, spotlight );

    float diffuseAttenuation = light.color.w / dot( attenuation, light.attenuation ) * spotlightAttenuation;
    float specularAttenuation = light.color.w / dot( attenuation, light.specularAttenuation ) * spotlightAttenuation;

    float incidentAngle = dot( toLightDirection, worldNormal );
    float facing = smoothstep( -.25, .1, incidentAngle );
    float diffuse = max( 0, incidentAngle );

    float3 viewDirection = normalize( CAMERA_POSITION - worldPosition );
    float3 halfDirection = normalize( lightDirection + viewDirection );
    float specular = max( 0, dot( worldNormal, halfDirection ) );

    specular = facing * specular;
    specular = pow( specular, SPECULAR_POWER );

    return float2(diffuseAttenuation * diffuse, specularAttenuation * specular);
}

float4 calculate_lighting_at_point( float3 worldPosition, float3 worldNormal, float4 textureColor, float4 tint )
{
    float3 surfaceColor = pow( textureColor.xyz, GAMMA ) * tint.xyz;
    float surfaceAlpha = tint.a * textureColor.a;

    float3 diffuse = pow( AMBIENT.xyz * AMBIENT.w, GAMMA.xxx );
    float3 specular = float3(0, 0, 0);

    for ( uint lightIndex = 0; lightIndex < MAX_LIGHTS; ++lightIndex )
    {
        float3 lightColor = LIGHTS[ lightIndex ].color.xyz;
        lightColor = pow( lightColor, GAMMA.xxx );

        float2 lightFactor = calculate_light_factors_at_point( worldPosition, worldNormal, LIGHTS[ lightIndex ] );

        diffuse += lightFactor.x * lightColor;
        specular += lightFactor.y * lightColor;
    }

    diffuse = saturate( diffuse );
    specular *= SPECULAR_FACTOR;

    float3 finalColor = diffuse * surfaceColor + specular;
    return float4(finalColor, 1);
}

float4 calculate_fog( float3 worldPosition, float4 inputColor)
{
    float distance = length( worldPosition - CAMERA_POSITION );

    float fogLerp = smoothstep( NEAR_FOG_DISTANCE, FAR_FOG_DISTANCE, distance );
    float4 fogColor = lerp( NEAR_FOG_COLOR, FAR_FOG_COLOR, fogLerp );

    float useFog = step( NEAR_FOG_DISTANCE, distance );

    float4 finalColor = lerp( inputColor, fogColor, fogLerp );
    return finalColor;
}

#endif // __LIGHTING_UTILS_HLSL__