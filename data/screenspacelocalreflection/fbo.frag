#version 150 core

in vec2 v_uv;

out vec4 fragColor;

uniform sampler2D fboTexture;

void main()
{
    fragColor = texture(fboTexture, v_uv);
	// fragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
