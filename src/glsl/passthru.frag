
// varying vec4 gl_TexCoord[0];

uniform sampler2D tex0;
varying highp vec2 vary_tex_cord;

void main()
{
	gl_FragColor = texture2D(tex0, vary_tex_cord);
}
