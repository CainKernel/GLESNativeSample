
// vertex shader
#version 300 es
layout(location = 0) in vec4 a_postion;
layout(location = 1) in vec2 a_texCoord;
out vec2 v_texCoord;
void main() {
    gl_Position = a_postion;
    v_texCoord = a_texCoord;
}