//  Copyright (c) 2010-2011 Mentor Graphics Corporation - All Rights Reserved.

// Attributes
attribute highp vec3 a_VertCoord;
attribute highp vec2 a_TexCoord;

// Uniforms
uniform highp mat4 u_ModelViewMatrix;
uniform highp mat4 u_ProjectionMatrix;

// Varyings
varying mediump vec2 v_TexCoord;

// main ()
void main (void)
{
    // Transform the vertex position via ModelView Projection transformation
    gl_Position = u_ProjectionMatrix * (u_ModelViewMatrix * vec4(a_VertCoord, 1.0) );

    // Pass along texture coordinates for fragment shader to use
    v_TexCoord = a_TexCoord;
}