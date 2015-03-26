#version 130

#ifdef GL_ES
#define varying out
#define attribute in
#endif
#line 6

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewProjectionMatrix;

uniform mediump vec2 texture_size;
uniform mediump vec2 video_window_size;

attribute highp vec4 attrVertex;
// varying vec4 gl_TexCoord[0];

varying highp vec2 vary_tex_cord;

// 假冒的 tex 地址，其实只是插值产生纹理数值的整数坐标
// varying vec2 fake_tex_cord;

void main()
{
	gl_Position = ModelViewProjectionMatrix* attrVertex;

	vec2 tex_cord;
	// 计算纹理坐标

	tex_cord.x = (attrVertex.x / video_window_size.x) * texture_size.x;
	tex_cord.y = (attrVertex.y / video_window_size.y) * texture_size.y;

	vec2 tcoord = (tex_cord * 2.0 );//+ vec2(1.0, 1.0) );

	tcoord.x /= (texture_size.x * 2.0) ;
	tcoord.y /= (texture_size.y * 2.0) ;

	vary_tex_cord = tcoord;
}
