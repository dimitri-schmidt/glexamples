#version 330 core

in vec2 v_uv;

out vec4 fragColor;


uniform mat4 transform;
uniform sampler2D fboTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform sampler2D positionTexture;
uniform vec3 eye;

void main()
{
    // fragColor = texture(fboTexture, v_uv);
	// fragColor = texture(normalTexture, v_uv);
    // fragColor = vec4(vec3(texture(depthTexture, v_uv).r), 1.0);
	// fragColor = texture(positionTexture, v_uv);

    vec3 position = texture(positionTexture, v_uv).xyz;
    vec3 view = normalize(position - eye);
    vec3 reflection = normalize(reflect(view, normalize(texture(normalTexture, v_uv).xyz)));


    vec3 vspPosReflect = eye + reflection;
    vec3 sspPosReflect = (vec4(vspPosReflect, 1.0) * transform).xyz / vspPosReflect.z;
    vec3 sspReflect = sspPosReflect - eye;

    fragColor = vec4(sspReflect, 1.0);
}
	
