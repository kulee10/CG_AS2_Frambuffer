#version 410

layout(location = 0) out vec4 fragColor;

uniform int DOG;
uniform int width_Dog;
uniform int height_Dog;
uniform int width;
uniform int bar;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform sampler2D tex;
uniform sampler2D tex_q;

float sigma_e = 2.0f;
float sigma_r = 2.8f;
float phi = 3.4f;
float tau = 0.99f;
float twoSigmaESquared = 2.0 * sigma_e * sigma_e;
float twoSigmaRSquared = 2.0 * sigma_r * sigma_r;
int halfWidth = int(ceil( 2.0 * sigma_r ));


void main()
{	
	if (DOG == 1) {
		vec2 sum = vec2(0.0);
		vec2 norm = vec2(0.0);
		for ( int i = -halfWidth; i <= halfWidth; ++i ) {
			for ( int j = -halfWidth; j <= halfWidth; ++j ) {
				float d = length(vec2(i,j));
				vec2 kernel= vec2( exp( -d * d / twoSigmaESquared ),
				exp( -d * d / twoSigmaRSquared ));
				vec4 c= texture(tex,vertexData.texcoord+vec2(i,j)/vec2(width_Dog, height_Dog));
				vec2 L= vec2(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
				norm += kernel;
				sum += kernel * L;
			}
		}
		sum /= norm;
		float H = 100.0 * (sum.x - tau * sum.y);
		float edge =( H > 0.0 )?1.0:2.0 *smoothstep(-2.0, 2.0, phi * H );
		vec3 texColor = texture(tex_q,vertexData.texcoord).rgb;

		if (bar == 1) {
			if (gl_FragCoord.x >= width+5)
				fragColor = vec4(texColor.r * edge, texColor.g * edge, texColor.b * edge,1.0 );
			else if (gl_FragCoord.x <= width-5)
				fragColor = texture(tex,vertexData.texcoord);
		}
	}
	else {
		fragColor = texture(tex,vertexData.texcoord);
	}
}