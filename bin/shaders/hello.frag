#version 130 // to support in/out instead of old-style varying

// the second input from vertex shader
in vec3 vertex_color;

// define pixel output variable instead of old-style gl_FragColor
out vec4 fragColor;

// shader's global variables, called the uniform variables
uniform bool bUseSolidColor;
uniform vec4 solid_color;

void main()
{
	fragColor = bUseSolidColor ? solid_color : vec4(vertex_color,1);
}
