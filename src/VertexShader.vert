#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexPos;

out vec2 texPos;

out vec3 pos;
out vec3 projPos; //projected position

uniform mat4 projection;
uniform mat4 view;
uniform vec3 camPos;

void main() {
	vec4 currPos = projection * view * vec4(aPos.xyz - camPos, 1);
	gl_Position = currPos;
	texPos = aTexPos;
	//pos = currPos.xyz;
	projPos = currPos.xyz;
	pos = (view * vec4(aPos.xyz - camPos, 1)).xyz;
}