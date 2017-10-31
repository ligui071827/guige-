//  Copyright (c) 2010-2011 Mentor Graphics Corporation - All Rights Reserved.

precision mediump   float;

// Constants
const   float C_FLOAT_ZERO = 0.0;
const   float C_FLOAT_ONE  = 1.0;

// Varyings
varying   mediump vec3  v_Normal;
varying   mediump vec3  v_VPpli_0;
varying   mediump vec3  v_VPpli_1;
varying   mediump float v_Attenuation;
varying   lowp    vec4  v_LightColor;

// Uniforms

// Material Properties
uniform   vec4  u_MaterialSpecularColor;
uniform   vec4  u_MaterialDiffuseColor;
uniform   float u_MaterialSpecularExponent;

// Light Properties
uniform   vec4  u_LightDiffuseColor_0;
uniform   vec4  u_LightDiffuseColor_1;

uniform   vec4  u_LightSpecularColor_1;

// main ()
void main(void)
{
    vec4 light_color;
    vec3 VPpli;
    vec3 half_vector;
    vec4 computed_color;
    vec4 specular_color = vec4 (C_FLOAT_ZERO);
    vec3 normal;
    float NdotL;
    float pf = C_FLOAT_ZERO;

    // Normalize the normal now as interpolation has taken place
    normal = normalize (v_Normal);

    // Compute Light color
    light_color = v_LightColor;

    //-------------------------------------------------------
    // Primary Light
    //-------------------------------------------------------
    VPpli = normalize(v_VPpli_0);

    light_color += max (C_FLOAT_ZERO, dot(normal, VPpli) ) * u_LightDiffuseColor_0 * u_MaterialDiffuseColor;

    //-------------------------------------------------------
    // Secondary Light 1
    //-------------------------------------------------------
    VPpli = normalize (v_VPpli_1);

    // Specular light contribution (Half vector)
    half_vector = normalize (VPpli + vec3 (C_FLOAT_ZERO, C_FLOAT_ZERO, C_FLOAT_ONE));

    // Diffuse light contribution
    NdotL = max (C_FLOAT_ZERO, dot(normal, VPpli) );
    computed_color = NdotL * u_LightDiffuseColor_1 * u_MaterialDiffuseColor;

    // Specular light contribution
    if (NdotL != C_FLOAT_ZERO)
    {
        pf = pow (max (C_FLOAT_ZERO, dot (normal, half_vector) ), u_MaterialSpecularExponent);
    }

    specular_color = pf * u_LightSpecularColor_1 * u_MaterialSpecularColor;

    light_color += (computed_color * v_Attenuation);

    // Clamp to range [0.0, 1.0]
    light_color = clamp(light_color, C_FLOAT_ZERO, C_FLOAT_ONE);
    light_color.a = u_MaterialDiffuseColor.a;

    // Compute the fragment color and then add specular contribution
    gl_FragColor = light_color + vec4 (specular_color.rgb, 0.0);
}
