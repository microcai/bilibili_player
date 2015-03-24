#version 130
#line 2
uniform mediump vec2 texture_size;
uniform mediump vec2 video_window_size;

uniform sampler2D texY; // Y
uniform sampler2D texUV; // U


// YUV offset
// const vec3 offset = vec3(-0.0625, -0.5, -0.5);
const mediump vec3 offset = vec3(-0.0625, -0.5, -0.5);
// RGB coefficients
const mediump vec3 Rcoeff = vec3(1.164,  0.000,  1.596);
const mediump vec3 Gcoeff = vec3(1.164, -0.391, -0.813);
const mediump vec3 Bcoeff = vec3(1.164,  2.018,  0.000);

varying highp vec2 vary_tex_cord;

mediump vec3 color_tweak(in mediump vec3 yuv);
mediump vec3 yuv2rgb(in mediump vec3 yuv);

mediump vec3 get_yuv_from_texture(in mediump vec2 tcoord)
{
	mediump vec3 yuv;
	float uv_int;

	yuv.x = texture2D(texY, tcoord).r;

	vec2 XY_even_odd = vary_tex_cord * texture_size;

	int XY_even_odd_x_rounded = int(round(XY_even_odd.x));

	float little_diff = XY_even_odd.x - XY_even_odd_x_rounded ;

	// Get the U and V values
	int real_U_x = (XY_even_odd_x_rounded) & 0xFFFFFFFE;
	int real_V_x = XY_even_odd_x_rounded | 1;

	vec2 tex_cord_U, tex_cord_V;

	tex_cord_U.y = tex_cord_V.y = vary_tex_cord.y;

	tex_cord_U.x = float(real_U_x) / texture_size.x;
	tex_cord_V.x = float(real_V_x) / texture_size.x;

	tex_cord_U.x += 0.2 / texture_size.x;
	tex_cord_V.x += 0.2 / texture_size.x;

// 	tex_cord_U.x = vary_tex_cordY.x;
// 	tex_cord_V.x = vary_tex_cordY.x;

	float U = texture2D(texUV, tex_cord_U).x;
	float V = texture2D(texUV, tex_cord_V).x;

	yuv.y = ( float(U)); /// 4294967295.0);// - 0.5 ;//texture2D(tex1, tcoord).r;

	yuv.z = (float(V)); /// 15.0);// - 0.5 ;//texture2D(tex2, tcoord).r;

	return yuv;

  	return vec3( yuv.z , 0.5,0.5);

	return vec3(yuv.yz, 0.0);
}

mediump vec4 mytexture2D(in mediump vec2 tcoord)
{
	mediump vec3 rgb, yuv;

	// 	float tex_cord_x = 2I + 1 / 2N

	yuv = get_yuv_from_texture(tcoord);

	// Do the color transform
	rgb = yuv2rgb(color_tweak(yuv));
	return mediump vec4(rgb, 1.0);
}

void main()
{
	// That was easy. :)
	gl_FragColor = mytexture2D(vary_tex_cord);
}
