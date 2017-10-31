//  Copyright (c) 2010-2011 Mentor Graphics Corporation - All Rights Reserved.

// Constants
const     float C_FLOAT_ZERO = 0.0;
const     float C_FLOAT_ONE  = 1.0;

// Attributes
attribute highp vec3  a_VertCoord;
attribute highp vec3  a_NormalCoord;

// Uniforms

// Transformation Matrices
uniform   highp mat4  u_ModelViewMatrix;
uniform   highp mat4  u_ProjectionMatrix;
uniform   highp mat3  u_NormalMatrix;

// Material Properties
uniform   vec4  u_MaterialEmissiveColor;
uniform   vec4  u_MaterialSceneColor;
uniform   vec4  u_MaterialAmbientColor;

// Light Properties
uniform   vec4  u_LightAmbientColor_0;
uniform   vec4  u_LightAmbientColor_1;

uniform   vec4  u_LightPosition_0;
uniform   vec4  u_LightPosition_1;

uniform   vec3  u_LightAttenuationFactors_1;

// Varyings
varying   mediump vec3  v_Normal;
varying   mediump vec3  v_VPpli_0;
varying   mediump vec3  v_VPpli_1;
varying   lowp    vec4  v_LightColor;
varying   mediump float v_Attenuation; // x-component for light 1 and y-component for light 2

// main ()
void main (void)
{
    float att_factor = C_FLOAT_ONE;
    vec3  att_dist;
    vec4  eye_position;

    // Transform the vertex position via current ModelView matrix
    eye_position = u_ModelViewMatrix * vec4(a_VertCoord, 1.0);

    // Transform the vertex position via ModelView Projection transformation
    gl_Position = u_ProjectionMatrix * eye_position;

    // Transform normal via Normal matrix and leave normalization for fragment shader
    v_Normal = u_NormalMatrix * a_NormalCoord;

    v_LightColor = (u_MaterialEmissiveColor) + (u_MaterialAmbientColor * u_MaterialSceneColor);

    //-------------------------------------------------------
    // Primary Light
    //-------------------------------------------------------
    v_VPpli_0 = u_LightPosition_0.xyz;
    v_LightColor += (u_LightAmbientColor_0 * u_MaterialAmbientColor);

    //-------------------------------------------------------
    // Secondary Light 1
    //-------------------------------------------------------
    v_VPpli_1 = u_LightPosition_1.xyz - eye_position.xyz;

    // Take into account the distance attenuation
    att_dist.x = C_FLOAT_ONE;
    att_dist.z = dot (v_VPpli_1, v_VPpli_1);
    att_dist.y = sqrt (att_dist.z);

    att_factor = C_FLOAT_ONE / dot (att_dist, u_LightAttenuationFactors_1);

    v_Attenuation = att_factor;

    v_LightColor += (att_factor * u_LightAmbientColor_1 * u_MaterialAmbientColor);
}
