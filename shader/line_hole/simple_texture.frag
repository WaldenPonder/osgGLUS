#version 430 compatibility
uniform sampler2D baseTexture;

in vec2 texcoord;
out vec4 fragColor;

void main()
{
	fragColor = texture(baseTexture, texcoord);
}

