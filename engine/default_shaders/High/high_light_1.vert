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
uniform   vec4  u_LightPosition_0;
uniform   vec4  u_LightAmbientColor_0;

// Varyings
varying   mediump vec3  v_Normal;
varying   mediump vec3  v_VPpli_0;
varying   lowp    vec4  v_LightColor;

// main ()
void main (void)
{
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
}
