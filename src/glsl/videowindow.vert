
/*
uniform mat4 gl_ModelViewMatrix;
uniform mat4 gl_ProjectionMatrix;
uniform mat4 gl_ModelViewProjectionMatrix;*/

// attribute vec4 gl_Vertex;
// varying vec4 gl_TexCoord[0];
uniform vec2 texture_size;
uniform vec2 video_window_size;


// 假冒的 tex 地址，其实只是插值产生纹理数值的整数坐标
// varying vec2 fake_tex_cord;

void main()
{
// 	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position = ftransform();

	vec2 tex_cord;
	// 计算纹理坐标

	tex_cord.x = (gl_Vertex.x / video_window_size.x) * texture_size.x;
	tex_cord.y = (gl_Vertex.y / video_window_size.y) * texture_size.y;

	vec2 tcoord = (tex_cord * 2.0 );//+ vec2(1.0, 1.0) );

	tcoord.x /= (texture_size.x * 2.0) ;
	tcoord.y /= (texture_size.y * 2.0) ;

	gl_TexCoord[0] = vec4(tcoord, 0.0, 0.0);

}
