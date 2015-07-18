#version 330 core

in vec2 v_uv;

out vec4 fragColor;


uniform mat4 transform;
uniform vec2 viewport;
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
    vec3 normal = normalize(texture(normalTexture, v_uv).xyz);

    
    vec4 reflection4 = transform * vec4(reflect(view, normal), 1.0);
    vec3 reflection = reflection4.xyz / reflection4.w;

    // fragColor = vec4(reflection * 0.5 + 0.5, 1.0);

    float scale = (2.0 / viewport.x) / length(reflection.xy);
    reflection *= scale;

    vec2 uv = v_uv;
    float expDepth = texture(depthTexture, v_uv).r;

    bool match = false;

    while(uv.x <= 1.0 &&  uv.x >= 0.0 && uv.y <= 1.0 && uv.y >= 0.0 && match == false)
    {
        uv += reflection.xy;
        expDepth += reflection.z;

        float newDepth = texture(depthTexture, uv).r;

        // float threshold = 0.1;

        // if(abs(depth - newDepth) < threshold)
        if(expDepth <= newDepth)
        {
            // fragColor = texture(fboTexture, uv);
            fragColor = vec4(1.0, 0.0, 1.0, 1.0);
            match = true;
        }
    }

    if(match==false){
        fragColor = texture(fboTexture, v_uv);
    }

    // fragColor = vec4(reflection * 0.5 + 0.5, 1.0);
}
	
