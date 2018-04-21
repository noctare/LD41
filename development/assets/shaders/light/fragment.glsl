#version 130

uniform sampler2D uni_Texture;

uniform vec2 uni_LightPosition[40];
uniform vec3 uni_LightColor[40];
uniform float uni_LightIntensity[40];
uniform int uni_LightCount;
uniform float uni_BaseLight;

in vec4 ex_Color;
in vec2 ex_TexCoords;
in vec4 ex_VertexPosition;

out vec4 out_Color;
	
vec3 apply_light(vec2 position, vec3 color, float intensity) {
	float distance = length(position - ex_VertexPosition.xy);
	intensity = 1.0f - distance * (1.0f - intensity);
	return color * max(intensity, uni_BaseLight);
}

void main() {
	vec3 lights = vec3(0.0f, 0.0f, 0.0f);
	vec4 base = ex_Color * texture(uni_Texture, ex_TexCoords);
	vec3 used_light = vec3(uni_BaseLight);
	for (int i = 0; i < uni_LightCount; i++) {
		vec3 light = apply_light(uni_LightPosition[i], uni_LightColor[i], uni_LightIntensity[i]);
		used_light.r = max(used_light.r, light.r);
		used_light.g = max(used_light.g, light.g);
		used_light.b = max(used_light.b, light.b);
	}
	lights = used_light;
	out_Color = min(vec4(base.rgb * lights, base.a), base);
}
