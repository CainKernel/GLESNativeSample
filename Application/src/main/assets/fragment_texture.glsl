// fragment shader
#version 300 es
precision mediump float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
// 声明一个类型为sampler2D的统一变量
// 采样器是用于从纹理贴图中读取的特殊统一变量
uniform sampler2D s_texture;
void main() {
    outColor = texture(s_texture, v_texCoord);
}