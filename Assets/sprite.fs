#version 330

in vec2 inTexCoord;

out vec4 color;

uniform sampler2D textureUniform;

void main() {
    color = texture(textureUniform, inTexCoord);
}
