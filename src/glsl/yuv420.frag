#version 130

#ifdef GL_ES
#define varying in
#endif
#line 6
#undef lowp
#undef mediump
#undef highp

uniform mediump vec2 texture_size;
uniform mediump vec2 video_window_size;

uniform sampler2D texY; // Y
uniform sampler2D texU; // U
uniform sampler2D texV; // U

varying highp vec2 vary_tex_cord;

mediump vec3 color_tweak(in mediump vec3 yuv);
mediump vec3 yuv2rgb(in mediump vec3 yuv);

mediump vec3 get_yuv_from_texture(in mediump vec2 tcoord)
{
	mediump vec3 yuv;

	yuv.x = texture(texY, tcoord).r;

	// Get the U and V values
	yuv.y = texture(texU, tcoord).r;

	yuv.z = texture(texV, tcoord).r;

	return yuv;
}

mediump vec4 mytexture2D(in mediump vec2 tcoord)
{
	mediump vec3 rgb, yuv;
// 	float tex_cord_x = 2I + 1 / 2N

	yuv = get_yuv_from_texture(tcoord);

	// Do the color transform
	rgb = yuv2rgb(color_tweak(yuv));
	return vec4(rgb, 1.0);
}

out highp vec4 out_color;

void main()
{
	// That was easy. :)
	out_color = mytexture2D(vary_tex_cord);
}
