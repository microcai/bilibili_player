
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

void main()
{
    vec3 rgb, yuv;
	vec2 tcoord = vec2(gl_TexCoord[0].st);

    yuv.x = texture2D(tex0, tcoord).r;

    // Get the U and V values
    yuv.y = texture2D(tex1, tcoord).r;
    yuv.z = texture2D(tex2, tcoord).r;

    // Do the color transform
    yuv += offset;
    rgb.r = dot(yuv, Rcoeff);
    rgb.g = dot(yuv, Gcoeff);
    rgb.b = dot(yuv, Bcoeff);

    // That was easy. :)
    gl_FragColor = vec4(rgb, 1.0);
   return;
    rgb.r = clamp( R /255.0 , 0.0, 1.0);
    rgb.g = clamp( G /255.0 , 0.0, 1.0);
    rgb.b = clamp( B /255.0 , 0.0, 1.0);
    // That was easy. :)
    gl_FragColor = vec4(rgb, 1.0);
}
