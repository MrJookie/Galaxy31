#version 330

layout (location = 0) in vec2 position_and_texcoords;

out vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4( position_and_texcoords, 0.0, 1.0 );

    inTexCoord = vec2(position_and_texcoords.x, position_and_texcoords.y);
}
