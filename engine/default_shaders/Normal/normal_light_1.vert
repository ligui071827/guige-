//  Copyright (c) 2010-2011 Mentor Graphics Corporation - All Rights Reserved.

// Constants
const float C_FLOAT_ZERO = 0.0;
const float C_FLOAT_ONE = 1.0;

// Attributes
attribute highp vec3 a_VertCoord;
attribute highp vec3 a_NormalCoord;

// Uniforms

// Transformation Matrices
uniform highp mat4 u_ModelViewMatrix;
uniform highp mat4 u_ProjectionMatrix;
uniform highp mat3 u_NormalMatrix;

// Material Properties
uniform vec4 u_MaterialAmbientColor;
uniform vec4 u_MaterialDiffuseColor;
uniform vec4 u_MaterialEmissiveColor;
uniform vec4 u_MaterialSceneColor;

// Light Properties
uniform vec4 u_LightAmbientColor_0;
uniform vec4 u_LightPosition_0;
uniform vec4 u_LightDiffuseColor_0;

// Varyings
varying lowp vec4 v_LightColor;

// main ()
void main (void)
{
    vec4  light_color;
    vec4  eye_position;
    vec3  normal;

    // Transform the vertex position via current ModelView matrix
    eye_position = u_ModelViewMatrix * vec4(a_VertCoord, 1.0);

    // Transform the vertex position via ModelView Projection transformation
    gl_Position = u_ProjectionMatrix * eye_position;

    // Transform the normal via Normal matrix and then normalize
    normal = normalize (u_NormalMatrix * a_NormalCoord);

    // Global Ambient level
    light_color = (u_MaterialEmissiveColor) + (u_MaterialAmbientColor * u_MaterialSceneColor);

    //-------------------------------------------------------
    // Primary Light
    //-------------------------------------------------------
    // Ambient light contribution
    light_color += (u_LightAmbientColor_0 * u_MaterialAmbientColor);

    // Diffuse light contribution
    light_color += max (C_FLOAT_ZERO, dot(normal, u_LightPosition_0.xyz) ) * u_LightDiffuseColor_0 * u_MaterialDiffuseColor;

    // Clamp to range [0.0, 1.0]
    light_color = clamp(light_color, C_FLOAT_ZERO, C_FLOAT_ONE);
    light_color.a = u_MaterialDiffuseColor.a;

    v_LightColor = light_color;
}
