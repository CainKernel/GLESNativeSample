#version 300 es
precision mediump float;

uniform vec4 u_fogColor;
uniform float u_fogMaxDist;
uniform float u_fogMinDist;
uniform sampler2D s_baseMap;

in vec2 v_texCoord;
in float v_eyeDist;

layout(location = 0) out vec4 outColor;

float computeLinearFogFactor() {
    float factor;
    // 线性雾化计算公式
    factor = (u_fogMaxDist - v_eyeDist) / (u_fogMinDist - u_fogMinDist);
    return factor;
}

void main() {
    float fogFactor = computeLinearFogFactor();
    vec4 baseColor = texture(s_baseMap, v_texCoord);
    // 根据得到的线性雾化因子，计算实际输出的雾化效果
    outColor = baseColor * vec4(fogFactor) + u_fogColor * (1.0 - fogFactor);
}
