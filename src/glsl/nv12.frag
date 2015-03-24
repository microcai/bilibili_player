
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

//brightness,对比度contrast,饱和度saturation 调整参数

uniform mediump float brightness;
uniform mediump float contrast;
uniform mediump float saturation;

varying mediump vec2 vary_tex_cord;

// 假冒的 tex 地址，其实只是插值产生纹理数值的整数坐标
// varying vec2 fake_tex_cord;

mediump vec3 yuv2rgb(in mediump vec3 yuv)
{
	mediump vec3 rgb;

	yuv = clamp(yuv, 0.0, 1.0);

	yuv += offset;

	rgb.r = dot(yuv, Rcoeff);
	rgb.g = dot(yuv, Gcoeff);
	rgb.b = dot(yuv, Bcoeff);
	return rgb;
}

mediump vec3 color_tweak(in mediump vec3 yuv)
{
	mediump float newY, newU, newV;

	newY = ((yuv.x-0.5) * contrast + 0.5) * brightness;
	newU = ((yuv.y-0.5) * saturation + 0.5);
	newV = ((yuv.z-0.5) * saturation + 0.5);

	return vec3(newY, newU, newV);
}

mediump vec3 get_yuv_from_texture(in mediump vec2 tcoord)
{
	mediump vec3 yuv;

	yuv.x = texture2D(texY, tcoord).r;

	// Get the U and V values
	yuv.y = 0.5;//texture2D(tex1, tcoord).r;

	yuv.z = 0.5;//texture2D(tex2, tcoord).r;

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
	gl_FragColor = vec4(1.0,0.0,0.1,1.0); //mytexture2D(vary_tex_cord);
}
