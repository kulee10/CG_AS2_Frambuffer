#version 410

layout(location = 0) out vec4 fragColor;

uniform int blur;
uniform int bar;
uniform int width;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform sampler2D tex;
uniform sampler2D tex_b;


void main()
{	
	if (blur == 1) {
		int half_size = 2;
		vec4 color_sum = vec4(0);
		for (int i = -half_size; i <= half_size ; ++i) {
			for (int j = -half_size; j <= half_size ; ++j) {
				ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
				color_sum += texelFetch(tex_b, coord, 0);
			}
		}
		int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
		fragColor = color_sum / sample_count;
	} 
	else if (blur == 2) {
		vec4 Color1 = texture(tex,vertexData.texcoord);
		vec4 Color2 = texture(tex_b,vertexData.texcoord);
		if (bar == 1) {
			if (gl_FragCoord.x >= width+5)
				fragColor = Color1 + Color2*0.6;
			else if (gl_FragCoord.x <= width-5)
				fragColor = texture(tex, vertexData.texcoord);
		}
	}
	else {
		fragColor = texture(tex, vertexData.texcoord);
	}
}