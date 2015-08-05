#version 330 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;

out vec3 v_normal;
out vec3 v_position;
out vec4 v_color;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 transform;
uniform mat4 translate;
uniform mat4 rotate;
uniform mat4 scale;

void main()
{
    vec4 position = transform * (vec4(a_vertex, 1.0) * scale * rotate * translate);
    // vec4 position = (vec4(a_vertex, 1.0) * scale * rotate * translate);
    v_position = position.xyz / position.w;
    gl_Position = position;
    // gl_Position = transform * position;
	// v_normal = vec3(vec4(a_normal, 1.0) * rotate).xyz;


    // vec4 normal = transform * (vec4(a_normal, 1.0) * rotate);
    

    // vec4 normal = (vec4(a_normal, 1.0) * rotate);    //right left have both red in it, dafuq??
    vec4 normal = (rotate * vec4(a_normal, 1.0));

    // normal = normal / normal.w;
    // normal = transform * normal;       // changes while zooming, think it shouldn't change
    normal = normal * transform;
    v_normal = normal.xyz;
    // v_normal = normal.xyz / normal.w;
    v_normal.z *= -1.0;

    v_color = vec4(a_normal * 0.5 + 0.5, 1.0);
}
