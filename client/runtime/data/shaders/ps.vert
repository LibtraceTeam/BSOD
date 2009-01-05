uniform vec3 fTime;

void main()
{	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
	
	vec3 offset = vec3(fTime[0], fTime[1], fTime[2]);
	gl_Position = gl_Position + vec4(offset.x, offset.y, offset.z, 0);
					
	gl_FrontColor = gl_Color;	
} 

