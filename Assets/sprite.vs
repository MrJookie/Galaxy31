#version 330

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

out vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4( position, 0.0, 1.0 );
    
    //inColor = color;
    inTexCoord = vec2(1.0 - texCoord.x, 1.0 - texCoord.y);
}
