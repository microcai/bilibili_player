
// varying vec4 gl_TexCoord[0];

uniform sampler2D tex0;

void main()
{
	gl_FragColor = texture2D(tex0, gl_TexCoord[0].st);
}
