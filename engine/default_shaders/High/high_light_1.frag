//  Copyright (c) 2010-2011 Mentor Graphics Corporation - All Rights Reserved.

precision mediump   float;

// Constants
const   float C_FLOAT_ZERO = 0.0;
const   float C_FLOAT_ONE  = 1.0;

// Varyings
varying   mediump vec3  v_Normal;
varying   mediump vec3  v_VPpli_0;
varying   lowp    vec4  v_LightColor;

// Uniforms

// Material Properties
uniform   vec4  u_MaterialDiffuseColor;

// Light Properties
uniform   vec4  u_LightDiffuseColor_0;

// main ()
void main(void)
{
    vec4  light_color;
    vec3  VPpli;
    vec3  normal;

    // Normalize the normal now as interpolation has taken place
    normal = normalize (v_Normal);

    // Compute Light color
    light_color = v_LightColor;

    //-------------------------------------------------------
    // Primary Light
    //-------------------------------------------------------
    VPpli = v_VPpli_0;

    // Light 0 diffuse contribution
    light_color += max (C_FLOAT_ZERO, dot(normal, VPpli) ) * u_LightDiffuseColor_0 * u_MaterialDiffuseColor;

    // Clamp to range [0.0, 1.0]
    light_color = clamp(light_color, C_FLOAT_ZERO, C_FLOAT_ONE);
    light_color.a = u_MaterialDiffuseColor.a;

    // Compute the fragment color without specular contribution
    gl_FragColor = light_color;
}