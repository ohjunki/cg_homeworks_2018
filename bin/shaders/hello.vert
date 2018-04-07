#version 130 // to support in/out instead of old-style varying

// input attributes of vertices
in vec3 position;
in vec3 normal;		// we are using this for vertex color

// the second output = input to fragment shader
out vec3 vertex_color;

// uniform variables
uniform vec3 circlePosition;
uniform mat3 resultionConvert;
uniform float circleScaler;

void main()
{
	// gl_Position: a built-in output variable that should be written in main()
	gl_Position = vec4( ( position*circleScaler+circlePosition )*resultionConvert , 1 );
	// another output passed via varying variable
	vertex_color = normal;	// pass the color in norm to the vertex color output
}