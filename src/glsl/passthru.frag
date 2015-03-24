#version 130

#ifdef GL_ES
#define varying in
#endif
#line 6

#undef lowp
#undef mediump
#undef highp

// varying vec4 gl_TexCoord[0];
#ifdef GL_ES
out mediump vec4 out_color;
#else
#define out_color gl_FragColor
#endif

uniform sampler2D tex0;
out highp vec2 vary_tex_cord;

void main()
{
	out_color = texture(tex0, vary_tex_cord);
}
