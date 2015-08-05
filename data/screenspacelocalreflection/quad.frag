#version 330 core

in vec2 v_uv;

out vec4 fragColor;

uniform float treshold;
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

		vec3 view = normalize(ssEye - position);

		vec3 normal = normalize(texture(normalTexture, v_uv).xyz * 2.0 - 1.0);
		
		// vec4 reflection4 = transform * vec4(reflect(view, normal), 1.0);
		// vec3 reflection = reflection4.xyz / reflection4.w;
		vec3 reflection = reflect(-view, normal);         // -view, da vom eye zur oberfläche

        // fragColor = vec4(reflection, 1.0);

		float scale = (2.0 / viewport.x) / length(reflection.xy);
		reflection *= scale;
        // reflection.z *= -1.0;

        vec2 uv = v_uv + reflection.xy;
        float expDepth = texture(depthTexture, v_uv).r + reflection.z;
		float newDepth = 0.0;

		
   //      while(uv.x <= 1.0 && uv.x >= 0.0 && uv.y <= 1.0 && uv.y >= 0.0)
   //      {
   //          newDepth = texture(depthTexture, uv).r;

   //          float diff = expDepth - newDepth;

   //          // if(expDepth < newDepth) //> 0.0 && expDepth - newDepth < 0.002)
   //           if(diff > 0.0 && diff < treshold)
   //          //if (newDepth < expDepth)
   //          {
   //              // fragColor = mix(texture(fboTexture, v_uv), texture(fboTexture, uv), 0.9);
   //              fragColor = mix(vec4(1.0, 1.0, 1.0, 1.0), texture(fboTexture, uv), 0.7);
   //              // fragColor = vec4(1.0, 0.0, 1.0, 1.0);
   //              break;
   //          }
			// uv += reflection.xy;
   //          expDepth += reflection.z;
   //      }
    }

    gl_FragDepth = texture(depthTexture, v_uv).r;

}
	
