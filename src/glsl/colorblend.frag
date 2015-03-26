#version 130

#ifdef GL_ES
#define varying in
#endif
#line 6

#undef lowp
#undef mediump
#undef highp

uniform sampler2D tex0;
uniform highp vec4 color;
varying highp vec2 vary_tex_cord;

void main()
{
	gl_FragColor = vec4(color.rgb, texture(tex0, vary_tex_cord).x);
}
