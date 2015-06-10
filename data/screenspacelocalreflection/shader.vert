#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;

out vec3 v_normal;

uniform mat4 transform;
uniform mat4 translate;

void main()
{
    vec4 a = vec4(a_vertex, 1.0) * translate;
	gl_Position = transform * a;
	v_normal = a_normal;
}
