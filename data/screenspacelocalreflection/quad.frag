#version 330 core

in vec2 v_uv;

out vec4 fragColor;

uniform float maxDepthDifference;
uniform float selfReflectionThreshold;
uniform mat4 transform;
uniform vec2 viewport;
uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform sampler2D positionTexture;
uniform sampler2D reflectivenessTexture;
uniform vec3 eye;

void main()
{
    vec4 originalColor = texture(colorTexture, v_uv);
    fragColor = originalColor;
    float reflectiveness = texture(reflectivenessTexture, v_uv).r;
	if(reflectiveness > 0.0)
	{
        vec3 position = texture(positionTexture, v_uv).xyz * 2.0 - 1.0;
		vec3 ssEye = (transform * vec4(eye, 1.0)).xyz;
		vec3 view = normalize(position - ssEye);
        vec3 normal = normalize(texture(normalTexture, v_uv).xyz * 2.0 - 1.0);
		
        vec3 reflection = reflect(view, normal);
        reflection.x *= viewport.x;
        reflection.y *= viewport.y;
        reflection = reflection / length(reflection.xy);

        vec2 pixel = vec2(v_uv.x * viewport.x, v_uv.y * viewport.y) + reflection.xy;
        ivec2 iPixel = ivec2(pixel);
        float expectedDepth = texelFetch(depthTexture, iPixel, 0).r - reflection.z;
        float actualDepth = 0.0;
        float lastMatchDiff = 0.0;
        float diff = 0.0;
        while(pixel.x >= 0.0 && pixel.x <= viewport.x && pixel.y >= 0.0 && pixel.y <= viewport.y)
        {
            actualDepth = texelFetch(depthTexture, iPixel, 0).r;
            diff = expectedDepth - actualDepth;
            if(diff > 0.0 && diff < maxDepthDifference && actualDepth < 1.0)
            {
                fragColor = mix(originalColor, texelFetch(colorTexture, iPixel, 0), reflectiveness);
                if(lastMatchDiff < diff - selfReflectionThreshold) break; 
                lastMatchDiff = max(diff, lastMatchDiff); 
            }
            pixel += reflection.xy;
            iPixel = ivec2(pixel);
            expectedDepth -= reflection.z;
        }
    }
    gl_FragDepth = texture(depthTexture, v_uv).r;
}