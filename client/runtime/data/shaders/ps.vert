uniform float fTime;

void main()
{	
	gl_TexCoord[0] = gl_MultiTexCoord0;
		
	vec3 offset = gl_Normal * (fTime - gl_TexCoord[0].x);
	vec4 offset4 = vec4(offset[0], offset[1], offset[2], 0.0);
	gl_Position = gl_ModelViewProjectionMatrix * (gl_Vertex + offset4);
						
	gl_FrontColor = gl_Color;	
} 

