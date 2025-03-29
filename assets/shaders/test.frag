#version 330 core

uniform sampler2D baseColorTexture;
uniform sampler2D overlayColorTexture;

in vec3 outColor;
out vec4 fragColor;

void main()
{
	fragColor = vec4(outColor, 1.0f);
}
