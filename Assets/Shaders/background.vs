#version 330

layout (location = 0) in vec2 position;

void main() {
	 //const vec2 quad_vertices[4] = vec2[4]( vec2( -1.0, -1.0), vec2( 1.0, -1.0), vec2( -1.0, 1.0), vec2( 1.0, 1.0) );
     //gl_Position = vec4(quad_vertices[gl_VertexID], 0.0, 1.0);
     
     gl_Position = vec4( position, 0.0, 1.0 );
}
