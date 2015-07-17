#version 330 core

in vec2 v_uv;

out vec4 fragColor;

uniform sampler2D fboTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

void main()
{
    fragColor = texture(fboTexture, v_uv);
    // fragColor = texture(depthTexture, v_uv);
	fragColor = texture(normalTexture, v_uv);
	// fragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
	