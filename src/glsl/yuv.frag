
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

    // Get the Y value
//     yuv.x = texture2D(tex0, tcoord).r;

    float Y, U, V;
    float R,G,B;

    Y = clamp( texture2D(tex0, tcoord).r * 255.0 , 0.0, 255.0);

    U = clamp( texture2D(tex1, tcoord).r * 255.0 , 0.0, 255.0);

    V = clamp( texture2D(tex2, tcoord).r * 255.0 , 0.0, 255.0);

    R = Y + 1.402*(V-128.0);

    G = Y - 0.344*(U-128.0) - 0.714 * (V-128.0);

    G = Y + 1.772*(U-128.0);

    yuv.x = texture2D(tex0, tcoord).r;

    // Get the U and V values
//     tcoord *= 0.5;
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
