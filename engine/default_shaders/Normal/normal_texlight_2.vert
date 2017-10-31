//  Copyright (c) 2010-2011 Mentor Graphics Corporation - All Rights Reserved.

// Constants
const float C_FLOAT_ZERO = 0.0;
const float C_FLOAT_ONE  = 1.0;

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

uniform vec4 u_LightPosition_0;
uniform vec4 u_LightPosition_1;

uniform vec4 u_LightDiffuseColor_0;
uniform vec4 u_LightDiffuseColor_1;

uniform vec4 u_LightSpecularColor_1;

uniform vec3 u_LightAttenuationFactors_1;

// Varyings
varying lowp    vec4 v_LightColor;
varying mediump vec2 v_TexCoord;

// main ()
void main (void)
{
    vec4  eye_position;
    vec3  normal;
    vec4  computed_color;
    vec3  VPpli;
    vec4  light_color;
    vec3  half_vector;
    float NdotL, NdotH;
    float att_factor = C_FLOAT_ONE;
    vec3  att_dist;

    // Transform the vertex position via current ModelView matrix
    eye_position = u_ModelViewMatrix * vec4(a_VertCoord, 1.0);

    // Transform the vertex position via ModelView Projection transformation
    gl_Position = u_ProjectionMatrix * eye_position;

    // Pass along texture coordinates for fragment shader to use
    v_TexCoord = a_TexCoord;

    // Transform the normal via Normal matrix and then normalize
    normal = normalize (u_NormalMatrix * a_NormalCoord);

    light_color = (u_MaterialEmissiveColor) + (u_MaterialAmbientColor * u_MaterialSceneColor);

    //-------------------------------------------------------
    // Primary Light
    //-------------------------------------------------------
    VPpli = u_LightPosition_0.xyz;

    // Ambient light contribution
    light_color += (u_LightAmbientColor_0 * u_MaterialAmbientColor);

    // Diffuse light contribution
    light_color += max (C_FLOAT_ZERO, dot(normal, VPpli) ) * u_LightDiffuseColor_0 * u_MaterialDiffuseColor;

    //-------------------------------------------------------
    // Secondary Light 1
    //-------------------------------------------------------
    VPpli = u_LightPosition_1.xyz - eye_position.xyz;

    // Take into account the distance attenuation
    att_dist.x = C_FLOAT_ONE;
    att_dist.z = dot (VPpli, VPpli);
    att_dist.y = sqrt (att_dist.z);
    att_factor = C_FLOAT_ONE / dot (att_dist, u_LightAttenuationFactors_1);

    VPpli = normalize (VPpli);

    // Ambient light contribution
    computed_color = (u_LightAmbientColor_1 * u_MaterialAmbientColor);

    // Diffuse light contribution
    NdotL = max (C_FLOAT_ZERO, dot(normal, VPpli) );
    computed_color +=  NdotL * u_LightDiffuseColor_1 * u_MaterialDiffuseColor;

    // Specular light contribution
    if (NdotL != C_FLOAT_ZERO)
    {
        half_vector = normalize (VPpli + vec3 (C_FLOAT_ZERO, C_FLOAT_ZERO, C_FLOAT_ONE));
        NdotH = max (C_FLOAT_ZERO, dot (normal, half_vector) );

        computed_color += pow (NdotH, u_MaterialSpecularExponent) * u_LightSpecularColor_1 * u_MaterialSpecularColor;
    }

    // Combine the contributing components
    light_color += (computed_color * att_factor);

    // Clamp to range [0.0, 1.0]
    light_color = clamp(light_color, C_FLOAT_ZERO, C_FLOAT_ONE);
    light_color.a = u_MaterialDiffuseColor.a;

    v_LightColor = light_color;
}
