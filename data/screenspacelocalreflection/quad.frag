#version 330 core

in vec2 v_uv;

out vec4 fragColor;

uniform sampler2D fboTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform sampler2D positionTexture;

void main()
{
    fragColor = texture(fboTexture, v_uv);
	// fragColor = texture(normalTexture, v_uv);
    // fragColor = vec4(vec3(texture(depthTexture, v_uv).r), 1.0);
	// fragColor = texture(positionTexture, v_uv);
}
	
