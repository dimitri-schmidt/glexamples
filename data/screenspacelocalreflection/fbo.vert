#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec2 a_vertex;

out vec2 v_vertex;


void main()
{
    v_vertex = a_vertex;
    gl_Position = vec4(a_vertex, 0.0, 1.0);
}
