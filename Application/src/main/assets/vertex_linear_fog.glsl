#version 300 es

uniform mat4 u_matViewProjection;
uniform mat4 u_matView;
uniform vec4 u_eyePos;

layout(location = 0) in vec4 a_vertex;
layout(location = 1) in vec2 a_texCoord0;

out vec2 v_texCoord;
out float v_eyeDist;

void main() {
    // 将vertex顶点转到view区域
    vec4 vViewPos = u_matView * a_vertex;
    // 计算眼睛距离
    v_eyeDist = sqrt((vViewPos.x - u_eyePos.x) * (vViewPos.x - u_eyePos.x)
                     + (vViewPos.y - u_eyePos.y) * (vViewPos.y - u_eyePos.y)
                     + (vViewPos.z - u_eyePos.z) * (vViewPos.z - u_eyePos.z));
    gl_Position = u_matViewProjection * a_vertex;
    v_texCoord = a_texCoord0.xy;
}
