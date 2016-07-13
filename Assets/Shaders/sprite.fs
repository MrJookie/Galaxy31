#version 330

in vec2 inTexCoord;

out vec4 color;

uniform sampler2D textureUniform;

void main() {
    color = texture(textureUniform, inTexCoord);
    //color = vec4(1.0, 1.0, 1.0, 1.0);
}
