void main()
{		
	output = gl_Color;
	
	vec4 base = texture2D(tex0, gl_TexCoord[0].st);
				
	//Set the final fragment output
	gl_FragColor = output;
}

