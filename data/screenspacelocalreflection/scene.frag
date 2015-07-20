#version 330 core

in vec3 v_normal;
in vec3 v_position;
in vec4 v_color;

layout (location=0) out vec4 fragColor;
layout (location=1) out vec3 fragNormal;
layout (location=2) out vec3 fragPosition;	

void main()
{
	fragColor = v_color;
	fragNormal = v_normal;
	fragPosition = v_position;
}
