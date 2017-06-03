#version 300 es

precision mediump float;

// 跟顶点着色器中的out vec3 v_color; 声明的变量一致
in vec3 v_color;
layout(location = 0) out vec4 fragColor;

void main(){
    fragColor = vec4(v_color, 1.0);
}