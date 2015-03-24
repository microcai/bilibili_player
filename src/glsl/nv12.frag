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
uniform sampler2D texUV; // U

uniform bool type_nv21; // NV12 or NV21

varying highp vec2 vary_tex_cord;

mediump vec3 color_tweak(in mediump vec3 yuv);
mediump vec3 yuv2rgb(in mediump vec3 yuv);

mediump vec3 get_yuv_from_texture(in mediump vec2 tcoord)
{
	mediump vec3 yuv;
	highp float uv_int;

	yuv.x = texture(texY, tcoord).r;

	mediump vec2 XY_even_odd = vary_tex_cord * texture_size;

	int XY_even_odd_x_rounded = int(round(XY_even_odd.x));

	mediump float little_diff = XY_even_odd.x - float(XY_even_odd_x_rounded);

	// Get the U and V values
	int real_U_x = (XY_even_odd_x_rounded) & 0xFFFFFFFE;
	int real_V_x = XY_even_odd_x_rounded | 1;

	mediump vec2 tex_cord_U, tex_cord_V;

	tex_cord_U.y = tex_cord_V.y = vary_tex_cord.y;

	tex_cord_U.x = float(real_U_x) / texture_size.x;
	tex_cord_V.x = float(real_V_x) / texture_size.x;

	tex_cord_U.x += 0.2 / texture_size.x;
	tex_cord_V.x += 0.2 / texture_size.x;

	highp float U = texture(texUV, tex_cord_U).x;
	highp float V = texture(texUV, tex_cord_V).x;

	if ( type_nv21 )
	{
		yuv.y = U;
		yuv.z = V;
	}else
	{
		yuv.y = V;
		yuv.z = U;
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

#ifdef GL_ES
out mediump vec4 out_color;
#else
#define out_color gl_FragColor
#endif

void main()
{
	// That was easy. :)
	out_color = mytexture2D(vary_tex_cord);
}

// NV12 end
