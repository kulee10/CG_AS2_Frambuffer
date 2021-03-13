#version 410

layout(location = 0) out vec4 fragColor;

uniform int pixel;
uniform int sinewave;
uniform int bar;
uniform int width;
uniform float mx;
uniform float my;
uniform int magnify;
uniform float Timer;
uniform int magnify_width;
uniform int magnify_height;
uniform float lenSize;
uniform int bar_click;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform sampler2D tex;

float LenSize = 0.2f;

void main()
{	
	if (pixel == 1) {
		vec3 tex_color;
		float Pixels = 900.0;
		float dx = 10.0 * (1.0 / Pixels);
		float dy = 10.0 * (1.0 / Pixels);
		vec2 pixel_Coord = vec2(dx * floor(vertexData.texcoord.x / dx), dy * floor(vertexData.texcoord.y / dy));
		tex_color = texture(tex, pixel_Coord).rgb;

		if (bar == 1) {
			if (gl_FragCoord.x >= width+5)
				fragColor = vec4(tex_color, 1.0);
			else if (gl_FragCoord.x <= width-5)
				fragColor = texture(tex,vertexData.texcoord);
		}
	} 
	else if (sinewave == 1) {
		vec2 UV = vec2(vertexData.texcoord.x + 0.01 * sin(vertexData.texcoord.y * 5 * 3.14 + Timer), vertexData.texcoord.y);
		if (bar == 1) {
			if (gl_FragCoord.x >= width+5)
				fragColor = texture(tex, UV);
			else if (gl_FragCoord.x <= width-5)
				fragColor = texture(tex,vertexData.texcoord);
		}
	}
	else {
		fragColor = texture(tex,vertexData.texcoord);
		if (magnify == 1) {
			vec2 screen = vec2(magnify_width, magnify_height);
			vec2 mouse = vec2(0, 0);
			if (my > 300)
				mouse = vec2(mx, 600-my);
			else
				mouse = vec2(mx, 600-my);
			vec2 FragCoord = gl_FragCoord.xy / screen.xy;
			vec2 m = mouse.xy / screen.xy;

			vec2 dist = FragCoord - m;
			float radius = sqrt(dot(dist, dist));

			vec2 UV;
			vec3 tex_color = vec3(0.0, 0.0, 0.0);
			if (radius > LenSize+0.005) 
			{
				tex_color = texture(tex, vertexData.texcoord).xyz;
			} 
			else if (radius < LenSize-0.005) 
			{
				UV = m + normalize(dist) * sin(radius * 3.14159 * 0.095);
				tex_color = texture(tex, vec2(UV.x, UV.y)).xyz;
			}
			fragColor = vec4(tex_color, 1.0);
		}
		if (gl_FragCoord.x > width-5 && gl_FragCoord.x < width+5)
			fragColor = vec4(1.0, 1.0, 0.0, 1.0);
	}
}