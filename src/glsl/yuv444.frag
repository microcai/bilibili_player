#version 130

#line 2
uniform mediump vec2 texture_size;
uniform mediump vec2 video_window_size;

uniform sampler2D texYUV;

varying highp vec2 vary_tex_cord;

mediump vec3 color_tweak(in mediump vec3 yuv);
mediump vec3 yuv2rgb(in mediump vec3 yuv);

uniform int yuv444_type;

mediump vec3 get_yuv_from_texture(in mediump vec2 tcoord)
{
	mediump vec3 yuv;
	switch(yuv444_type)
	{
		case 0: // QVideoFrame::Format_AYUV444
		case 1: // QVideoFrame::Format_AYUV444_Premultiplied
			yuv = texture2D(texYUV, tcoord).yzw;
			break;
		case 2: // QVideoFrame::Format_YUV444
			yuv = texture2D(texYUV, tcoord).xyz;
	}

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

void main()
{
	// That was easy. :)
	gl_FragColor = mytexture2D(vary_tex_cord);
}
