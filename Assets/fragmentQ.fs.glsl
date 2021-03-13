#version 410

layout(location = 0) out vec4 fragColor;

uniform int quantization;
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
uniform sampler2D tex_N;
uniform sampler2D tex2;


void main()
{	
	if (quantization == 1) {
		float nbins = 8.0;
		vec3 tex_color = texelFetch(tex, ivec2(gl_FragCoord.xy), 0).rgb;
		tex_color = floor(tex_color * nbins) / nbins;

		if (bar == 1) {
			if (gl_FragCoord.x >= width+5)
				fragColor = vec4(tex_color, 1.0);
			else if (gl_FragCoord.x <= width-5)
				fragColor = texture(tex2, vertexData.texcoord);
		}
	}
	else if (quantization == 2) {
		vec3 noiseColor = texture(tex_N, vertexData.texcoord).rgb;
		vec2 UV = vertexData.texcoord + noiseColor.rg * 0.03;
		fragColor = texture(tex, UV);
	}
	else {
		fragColor = texture(tex2,vertexData.texcoord);
	}
}