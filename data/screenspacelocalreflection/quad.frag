#version 330 core

in vec2 v_uv;

out vec4 fragColor;

uniform float maxDepthDifference;
uniform float selfReflectionThreshold;
uniform mat4 transform;
uniform vec2 viewport;
uniform sampler2D fboTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform sampler2D positionTexture;
uniform sampler2D reflectivenessTexture;
uniform vec3 eye;

void main()
{
  float reflectiveness = texture(reflectivenessTexture, v_uv).r;
	
	fragColor = texture(fboTexture, v_uv);

	if(reflectiveness > 0.0)
	{
        vec3 position = texture(positionTexture, v_uv).xyz * 2.0 - 1.0; 

		vec4 ssEye4 = transform * vec4(eye, 1.0);       //must be right, cauze next line
        // vec4 ssEye4 = vec4(eye, 1.0) * transform;    //changes color while moving!!!!!!
		
        vec3 ssEye = ssEye4.xyz;                        //ssEye = (0,0,-156)
        // vec3 ssEye = ssEye4.xyz / ssEye4.w;          //flackert

		vec3 view = normalize(ssEye - position);        //von oberfläche zum auge

        vec3 normal = normalize(texture(normalTexture, v_uv).xyz * 2.0 - 1.0);

		// vec4 reflection4 = transform * vec4(reflect(view, normal), 1.0);
		// vec3 reflection = reflection4.xyz / reflection4.w;
		vec3 reflection = reflect(-view, normal);         // -view, da vom eye zur oberfläche


        float scale = (2.0 / viewport.x) / length(reflection.xy);
        reflection *= scale;
        // reflection.z *= -1.0;

        // reflection.y = (reflection.y / abs(reflection.y)) - reflection.y;



        vec2 pixel = vec2(v_uv.x * viewport.x, v_uv.y * viewport.y);
        float expDepth = texelFetch(depthTexture, ivec2(pixel), 0).r;
        float newDepth = 0.0;

        reflection.x *= viewport.x;
        reflection.y *= viewport.y;

        pixel += reflection.xy;
        expDepth -= reflection.z;

        float oldHitDiff = 0.0;
        while(pixel.x >= 0.0 && pixel.x <= viewport.x && pixel.y >= 0.0 && pixel.y <= viewport.y)
        {
            float newDepth = texelFetch(depthTexture, ivec2(pixel), 0).r;

            float diff = expDepth - newDepth;

            if(diff > 0.0)
            {
                if(texelFetch(depthTexture, ivec2(pixel), 0).r < 1.0)
                {
                    fragColor = mix(texture(fboTexture, v_uv), texelFetch(fboTexture, ivec2(pixel), 0), reflectiveness);
                    if(oldHitDiff < diff - 0.01)
                    {
                        // fragColor = mix(texture(fboTexture, v_uv), texture(fboTexture, uv), 0.9);
                        break;
                    } 
                    oldHitDiff = diff; 
                }
            }

            pixel += reflection.xy;
            expDepth -= reflection.z;
        }


    }

    gl_FragDepth = texture(depthTexture, v_uv).r;

}