#version 150 core

in vec2 v_uv;

out vec4 fragColor;

uniform sampler2D textureColor;

void main()
{
	fragColor = vec4(v_uv, 0.0, 1.0);			//texture(textureColor, v_uv);
}
