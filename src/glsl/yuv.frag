
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

void main()
{
    vec3 rgb, yuv;
	vec2 tcoord = vec2(gl_TexCoord[0].st);

    yuv.x = texture2D(tex0, tcoord).r;

    // Get the U and V values
    yuv.y = texture2D(tex1, tcoord).r;

    yuv.z = texture2D(tex2, tcoord).r;

    float newY, newU, newV;

    newY = ((yuv.x-0.5) * contrast + 0.5) * brightness;
    newU = ((yuv.y-0.5) * saturation + 0.5);
    newV = ((yuv.z-0.5) * saturation + 0.5);

    yuv.x = clamp(newY, 0.0, 1.0);
    yuv.y = clamp(newU, 0.0, 1.0);
    yuv.z = clamp(newV, 0.0, 1.0);

    // Do the color transform
    yuv += offset;
    rgb.r = dot(yuv, Rcoeff);
    rgb.g = dot(yuv, Gcoeff);
    rgb.b = dot(yuv, Bcoeff);

    // That was easy. :)
    gl_FragColor = vec4(rgb, gl_Color.a) * gl_Color;
}
