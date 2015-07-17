#version 330 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;

out vec3 v_normal;
out vec4 v_position;

uniform mat4 transform;
uniform mat4 translate;
uniform mat4 rotate;
uniform mat4 scale;

void main()
{
    v_position = transform * (vec4(a_vertex, 1.0) * scale * rotate * translate);
    gl_Position = v_position;
	v_normal = vec3(vec4(a_normal, 1.0) * rotate).xyz;
}
