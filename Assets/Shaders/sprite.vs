#version 330

layout (location = 0) in vec2 position_and_texcoords;

out vec2 inTexCoord;

uniform mat3 model;
// uniform mat4 view;
// uniform mat4 projection;
uniform mat4 projection_view_matrix;

void main() {
	vec4 v = vec4( model * vec3(position_and_texcoords, 1.0), 1.0 );
    gl_Position = projection_view_matrix * v;

    inTexCoord = vec2(position_and_texcoords.x, position_and_texcoords.y);
}
