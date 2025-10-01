#version 330 core

out vec2 TexCoord;

void main() {
    vec2 pos[3] = vec2[3](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
    );
    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
    TexCoord = gl_Position.xy * 0.5 + 0.5;
}