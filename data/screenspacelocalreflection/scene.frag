#version 330 core

in vec3 v_normal;
in vec4 v_position;
in vec4 v_color;

uniform float reflectiveness;

layout (location=0) out vec4 fragColor;
layout (location=1) out vec3 fragNormal;
layout (location=2) out vec3 fragPosition;	
layout (location=3) out vec3 fragReflectiveness;

void main()
{
	fragColor = v_color;
	fragNormal = (v_normal + 1.0) * 0.5;
    fragPosition = (v_position.xyz / v_position.w + 1.0) * 0.5;
	fragReflectiveness = vec3(reflectiveness);
}
