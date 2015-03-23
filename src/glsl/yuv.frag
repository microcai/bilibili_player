
uniform vec2 texture_size;
uniform vec2 video_window_size;

uniform sampler2D tex0; // Y
uniform sampler2D tex1; // U
uniform sampler2D tex2; // V


// YUV offset
// const vec3 offset = vec3(-0.0625, -0.5, -0.5);
const vec3 offset = vec3(-0.0625, -0.5, -0.5);
// RGB coefficients
const vec3 Rcoeff = vec3(1.164,  0.000,  1.596);
const vec3 Gcoeff = vec3(1.164, -0.391, -0.813);
const vec3 Bcoeff = vec3(1.164,  2.018,  0.000);

//brightness,对比度contrast,饱和度saturation 调整参数

uniform float brightness;
uniform float contrast;
uniform float saturation;


// 假冒的 tex 地址，其实只是插值产生纹理数值的整数坐标
// varying vec2 fake_tex_cord;

vec3 yuv2rgb(vec3 yuv)
{
	vec3 rgb;

	yuv = clamp(yuv, 0.0, 1.0);

	yuv += offset;

	rgb.r = dot(yuv, Rcoeff);
	rgb.g = dot(yuv, Gcoeff);
	rgb.b = dot(yuv, Bcoeff);
	return rgb;
}

vec3 color_tweak(vec3 yuv)
{
	float newY, newU, newV;

	newY = ((yuv.x-0.5) * contrast + 0.5) * brightness;
	newU = ((yuv.y-0.5) * saturation + 0.5);
	newV = ((yuv.z-0.5) * saturation + 0.5);

	return vec3(newY, newU, newV);
}

vec3 get_yuv_from_texture(vec2 tcoord)
{
	vec3 yuv;

	yuv.x = texture2D(tex0, tcoord).r;

	// Get the U and V values
	yuv.y = texture2D(tex1, tcoord).r;

	yuv.z = texture2D(tex2, tcoord).r;

	return yuv;
}

void main()
{
	vec3 rgb, yuv;
// 	float tex_cord_x = 2I + 1 / 2N


	vec2 tcoord = gl_TexCoord[0].st;


	yuv = get_yuv_from_texture(tcoord);

	// Do the color transform
	rgb = yuv2rgb(color_tweak(yuv));

	// That was easy. :)
	gl_FragColor = vec4(rgb, gl_Color.a);
}
