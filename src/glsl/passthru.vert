#version 130

#ifdef GL_ES
#define varying out
#define attribute in
#endif
#line 6

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewProjectionMatrix;

attribute highp vec4 attrVertex;
attribute highp vec4 attri_cord;
// varying vec4 gl_TexCoord[0];

varying highp vec2 vary_tex_cord;

// 假冒的 tex 地址，其实只是插值产生纹理数值的整数坐标
// varying vec2 fake_tex_cord;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * attrVertex;

	vary_tex_cord = attri_cord.st;
}
