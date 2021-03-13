#version 410

layout(location = 0) out vec4 fragColor;

in vec3 normal;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform int State;
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


void main()
{	
	vec3 texColor = texture(tex,vertexData.texcoord).rgb;
	if (State == 0) {
		if (bar == 1) {
			if (gl_FragCoord.x >= width+5)
				fragColor = vec4(texColor, 1.0);
			else if (gl_FragCoord.x <= width-5)
				fragColor = vec4(texColor, 1.0);
		} 
	}
	else if (State == 1) {
		if (bar == 1) {
			if (gl_FragCoord.x >= width+5)
				fragColor = vec4(normal, 1.0);
			else if (gl_FragCoord.x <= width-5)
				fragColor = vec4(texColor, 1.0);
		} 
	} else {
		fragColor = vec4(texColor, 1.0);
	}
}