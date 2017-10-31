//  Copyright (c) 2010-2011 Mentor Graphics Corporation - All Rights Reserved.

// Constants
const     float C_FLOAT_ZERO = 0.0;
const     float C_FLOAT_ONE  = 1.0;

// Attributes
attribute highp vec3  a_VertCoord;
attribute highp vec3  a_NormalCoord;
attribute highp vec2  a_TexCoord;

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
uniform   vec4  u_LightAmbientColor_2;

uniform   vec4  u_LightPosition_0;
uniform   vec4  u_LightPosition_1;
uniform   vec4  u_LightPosition_2;

uniform   vec3  u_LightAttenuationFactors_1;
uniform   vec3  u_LightAttenuationFactors_2;

uniform   bool  u_LightEnableState_2;

// Varyings
varying   mediump vec3  v_Normal;
varying   mediump vec3  v_VPpli_0;
varying   mediump vec3  v_VPpli_1;
varying   mediump vec3  v_VPpli_2;
varying   mediump vec2  v_AttenuationFactors; // x-component for light 1 and y-component for light 2
varying   lowp    vec4  v_LightColor;
varying   mediump vec2  v_TexCoord;

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

    // Pass along texture coordinates for fragment shader to use
    v_TexCoord = a_TexCoord;

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
    // Point/Bulb light source
    if (u_LightPosition_1.w != C_FLOAT_ZERO)
    {
        v_VPpli_1 = u_LightPosition_1.xyz - eye_position.xyz;

        // Take into account the distance attenuation
        att_dist.x = C_FLOAT_ONE;
        att_dist.z = dot (v_VPpli_1, v_VPpli_1);
        att_dist.y = sqrt (att_dist.z);

        att_factor = C_FLOAT_ONE / dot (att_dist, u_LightAttenuationFactors_1);

        v_AttenuationFactors.x = att_factor;

        v_LightColor += (att_factor * u_LightAmbientColor_1 * u_MaterialAmbientColor);
    }
    else  // Directional light source
    {
        v_VPpli_1 = u_LightPosition_1.xyz;

        v_AttenuationFactors.x = C_FLOAT_ONE;

        v_LightColor += (u_LightAmbientColor_1 * u_MaterialAmbientColor);
    }

    //-------------------------------------------------------
    // Secondary Light 2
    //-------------------------------------------------------
    if (u_LightEnableState_2)
    {
        // Point/Bulb light source
        if (u_LightPosition_2.w != C_FLOAT_ZERO)
        {
            v_VPpli_2 = u_LightPosition_2.xyz - eye_position.xyz;

            // Take into account the distance attenuation
            att_dist.x = C_FLOAT_ONE;
            att_dist.z = dot (v_VPpli_2, v_VPpli_2);
            att_dist.y = sqrt (att_dist.z);

            att_factor = C_FLOAT_ONE / dot (att_dist, u_LightAttenuationFactors_2);

            v_AttenuationFactors.y = att_factor;

            v_LightColor += (att_factor * u_LightAmbientColor_2 * u_MaterialAmbientColor);
        }
        else  // Directional light source
        {
            v_VPpli_2 = u_LightPosition_2.xyz;

            v_AttenuationFactors.y = C_FLOAT_ONE;

            v_LightColor += (u_LightAmbientColor_2 * u_MaterialAmbientColor);
        }
    }
}
