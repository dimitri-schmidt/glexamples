#version 330 core

in vec2 v_uv;

out vec4 fragColor;
// out float gl_FragDepth;

uniform mat4 view;
uniform mat4 projection;
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
    // fragColor = texture(fboTexture, v_uv);
	// fragColor = texture(normalTexture, v_uv);
    // fragColor = vec4(vec3(texture(depthTexture, v_uv).r), 1.0);
	// fragColor = texture(positionTexture, v_uv);

	float reflectiveness = texture(reflectivenessTexture, v_uv).r;
	
	fragColor = texture(fboTexture, v_uv);
	
	if(reflectiveness > 0.0)
	{	
		vec3 position = texture(positionTexture, v_uv).xyz;
		
		// vec3 view = normalize(position - eye);
		vec4 ssEye4 = view * vec4(eye, 1.0);
		vec3 ssEye = ssEye4.xyz; // / ssEye4.w;
		// vec3 view = normalize(position - ssEye);
		vec3 view = normalize(position-ssEye);
		vec3 normal = normalize(texture(normalTexture, v_uv).xyz);

		// fragColor = vec4(position, 1.0);

		//fragColor = vec4(view, 1.0);

		
		// vec4 reflection4 = transform * vec4(reflect(view, normal), 1.0);
		// vec3 reflection = reflection4.xyz / reflection4.w;
		vec3 reflection = reflect(view, normal);
		reflection = vec3(projection * vec4	(reflection, 1.0));

		// fragColor = vec4(reflection * 0.5 +0.5, 1.0);

		float scale = (2.0 / viewport.x) / length(reflection.xy);
		reflection *= scale;

        vec2 uv = v_uv + reflection.xy;
        float expDepth = texture(depthTexture, v_uv).r + reflection.z;
		float newDepth = 0.0;
		
        while(uv.x <= 1.0 &&  uv.x >= 0.0 && uv.y <= 1.0 && uv.y >= 0.0)
        {
            newDepth = texture(depthTexture, uv).r;

            if(expDepth - newDepth > 0.0 && expDepth - newDepth < 0.002)
            {
                fragColor = mix(texture(fboTexture, v_uv), texture(fboTexture, uv), reflectiveness);
                // fragColor = vec4(1.0, 0.0, 1.0, 1.0);
                break;
            }
			uv += reflection.xy;
            expDepth += reflection.z;
        }
    }
			//if(v_uv.x > 0.5)
				//fragColor = vec4(1.0, 0.0, 1.0, 1.0);

 //    fragColor = texture(fboTexture, v_uv);
	
	// if( reflectiveness > 0.0)
	// {
	// 	vec2 pixel = vec2(v_uv.x * viewport.x, v_uv.y * viewport.y);
	// 	float expDepth = texelFetch(depthTexture, ivec2(pixel), 0).r;

	// 	reflection.x *= viewport.x;
	// 	reflection.y *= viewport.y;

	// 	while(pixel.x >= 0.0 && pixel.x <= viewport.x && pixel.y >= 0.0 && pixel.y <= viewport.y)
	// 	{
	// 		pixel += reflection.xy;
	// 		expDepth += reflection.z;

	// 		float newDepth = texelFetch(depthTexture, ivec2(pixel), 0).r;

	// 		if(expDepth >= newDepth)
	// 		{
	// 			fragColor = mix(texture(fboTexture, v_uv), texelFetch(fboTexture, ivec2(pixel), 0), reflectiveness);
	// 			// fragColor = vec4(1.0, 0.0, 1.0, 1.0);
	// 			break;
	// 		}
	// 	}
	// }

    gl_FragDepth = texture(depthTexture, v_uv).r;

}
	
