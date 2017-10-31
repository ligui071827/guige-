//  Copyright (c) 2010-2011 Mentor Graphics Corporation - All Rights Reserved.

// Constants
const float C_FLOAT_ZERO = 0.0;
const float C_FLOAT_ONE = 1.0;

// Attributes
attribute highp vec3 a_VertCoord;
attribute highp vec3 a_NormalCoord;
attribute highp vec2 a_TexCoord;

// Uniforms

// Transformation Matrices
uniform highp mat4 u_ModelViewMatrix;
uniform highp mat4 u_ProjectionMatrix;
uniform highp mat3 u_NormalMatrix;

// Material Properties
uniform vec4  u_MaterialSpecularColor;
uniform vec4  u_MaterialAmbientColor;
uniform vec4  u_MaterialDiffuseColor;
uniform vec4  u_MaterialEmissiveColor;
uniform vec4  u_MaterialSceneColor;
uniform float u_MaterialSpecularExponent;

// Light Properties
uniform vec4 u_LightAmbientColor_0;
uniform vec4 u_LightAmbientColor_1;
uniform vec4 u_LightAmbientColor_2;

uniform vec4 u_LightPosition_0;
uniform vec4 u_LightPosition_1;
uniform vec4 u_LightPosition_2;

uniform vec4 u_LightDiffuseColor_0;
uniform vec4 u_LightDiffuseColor_1;
uniform vec4 u_LightDiffuseColor_2;

uniform vec4 u_LightSpecularColor_1;
uniform vec4 u_LightSpecularColor_2;

uniform vec3 u_LightAttenuationFactors_1;
uniform vec3 u_LightAttenuationFactors_2;

uniform bool u_LightEnableState_2;

// Varyings
varying mediump vec2 v_TexCoord;
varying lowp    vec4 v_LightColor;

// main ()
void main (void)
{
    vec4  eye_position;
    float NdotL, NdotH;
    vec3  half_vector;
    vec4  light_color;
    float att_factor;
    vec3  VPpli;
    vec4  computed_color;

    // Transform the vertex position via current ModelView matrix
    eye_position = u_ModelViewMatrix * vec4(a_VertCoord, 1.0);

    // Transform the vertex position via ModelView Projection transformation
    gl_Position = u_ProjectionMatrix * eye_position;

    // Transform the normal via Normal matrix and then normalize
    vec3 normal = normalize (u_NormalMatrix * a_NormalCoord);

    // Global Ambient level
    light_color = (u_MaterialEmissiveColor) + (u_MaterialAmbientColor * u_MaterialSceneColor);

    //-------------------------------------------------------
    // Primary Light
    //-------------------------------------------------------
    // Ambient light contribution
    light_color += (u_LightAmbientColor_0 * u_MaterialAmbientColor);

    // Diffuse light contribution
    light_color += max (C_FLOAT_ZERO, dot(normal, u_LightPosition_0.xyz) ) * u_LightDiffuseColor_0 * u_MaterialDiffuseColor;

    //-------------------------------------------------------
    // Secondary Light 1
    //-------------------------------------------------------
    // Point/Bulb light source
    if (u_LightPosition_1.w != C_FLOAT_ZERO)
    {
        vec3 att_dist;

        VPpli = (u_LightPosition_1 - eye_position).xyz;

        // Take into account the distance attenuation
        att_dist.x = C_FLOAT_ONE;
        att_dist.z = dot (VPpli, VPpli);
        att_dist.y = sqrt (att_dist.z);
        att_factor = C_FLOAT_ONE / dot (att_dist, u_LightAttenuationFactors_1);

        VPpli = normalize (VPpli);
    }
    else  // Directional light source
    {
        VPpli = u_LightPosition_1.xyz;
        att_factor = C_FLOAT_ONE;
    }

    // Ambient light contribution
    computed_color = (u_LightAmbientColor_1 * u_MaterialAmbientColor);

    // Diffuse light contribution
    NdotL = max (C_FLOAT_ZERO, dot(normal, VPpli) );
    computed_color += NdotL * u_LightDiffuseColor_1 * u_MaterialDiffuseColor;

    if (NdotL != C_FLOAT_ZERO)
    {
        // Specular light contribution
        half_vector = normalize (VPpli + vec3 (C_FLOAT_ZERO, C_FLOAT_ZERO, C_FLOAT_ONE));

        NdotH = max(dot (normal, half_vector), C_FLOAT_ZERO);

        computed_color += pow (NdotH, u_MaterialSpecularExponent) * u_LightSpecularColor_1 * u_MaterialSpecularColor;
    }

    light_color += (computed_color * att_factor);

    //-------------------------------------------------------
    // Secondary Light 2
    //-------------------------------------------------------
    if (u_LightEnableState_2)
    {
        // Point/Bulb light source
        if (u_LightPosition_2.w != C_FLOAT_ZERO)
        {
            vec3 att_dist;

            VPpli = (u_LightPosition_2 - eye_position).xyz;

            // Take into account the distance attenuation
            att_dist.x = C_FLOAT_ONE;
            att_dist.z = dot (VPpli, VPpli);
            att_dist.y = sqrt (att_dist.z);
            att_factor = C_FLOAT_ONE / dot (att_dist, u_LightAttenuationFactors_2);

            VPpli = normalize (VPpli);
        }
        else  // Directional light source
        {
            VPpli = u_LightPosition_2.xyz;
            att_factor = C_FLOAT_ONE;
        }

        // Ambient light contribution
        computed_color = (u_LightAmbientColor_2 * u_MaterialAmbientColor);

        // Diffuse light contribution
        NdotL = max (C_FLOAT_ZERO, dot(normal, VPpli) );
        computed_color += NdotL * u_LightDiffuseColor_2 * u_MaterialDiffuseColor;

        if (NdotL != C_FLOAT_ZERO)
        {
            // Specular light contribution
            half_vector = normalize (VPpli + vec3 (C_FLOAT_ZERO, C_FLOAT_ZERO, C_FLOAT_ONE));

            NdotH = max(dot (normal, half_vector), C_FLOAT_ZERO);

            computed_color += pow (NdotH, u_MaterialSpecularExponent) * u_LightSpecularColor_2 * u_MaterialSpecularColor;
        }

        light_color += (computed_color * att_factor);
    }

    // Clamp to range [0.0, 1.0]
    light_color = clamp(light_color, C_FLOAT_ZERO, C_FLOAT_ONE);
    light_color.a = u_MaterialDiffuseColor.a;

    v_LightColor = light_color;

    // Pass along texture coordinates for fragment shader to use
    v_TexCoord = a_TexCoord;
}
