#version 330 core

in vec3 v_normal;

layout (location=0) out vec4 fragColor;
layout (location=1) out vec4 fragNormal;

void main()
{
	fragColor = vec4(v_normal * 0.5 + 0.5, 1.0);
	fragNormal = vec4(v_normal, 1.0);
}
