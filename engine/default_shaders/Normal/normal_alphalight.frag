//  Copyright (c) 2010-2011 Mentor Graphics Corporation - All Rights Reserved.

precision mediump float;

// Varyings
varying lowp    vec4 v_LightColor;
varying mediump vec2 v_TexCoord;

// Uniforms
uniform lowp sampler2D s_Texture;

// main()
void main(void)
{
    // Modulate alpha texture with lighting color
    gl_FragColor = vec4 (v_LightColor.rgb, v_LightColor.a * texture2D (s_Texture, v_TexCoord).a);
}