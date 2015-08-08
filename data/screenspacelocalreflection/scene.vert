#version 330 core

in vec3 a_vertex;
in vec3 a_normal;

out vec3 v_normal;
out vec4 v_position;
out vec4 v_color;

uniform mat4 transform;
uniform mat4 translate;
uniform mat4 rotate;
uniform mat4 scale;

void main()
{
    vec4 position = transform * translate * rotate * scale * vec4(a_vertex, 1.0);
    v_position = position;
    gl_Position = position;
	
    v_normal = (vec4(a_normal, 1.0) * inverse(transform * rotate)).xyz;

    v_color = vec4((a_normal + 1.0) * 0.5, 1.0);
}
