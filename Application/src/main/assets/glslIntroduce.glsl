// 声明版本
#version 300 es

// OpenGLES着色器语言

// 默认精度
precision highp float;
precision mediump int;

// 精度限定符
highp vec4 position;
varying lowp vec4 color;
mediump float specularExp;

// 统一变量Uniform
// 统一变量是类型限定符，统一变量存储应用程序通过OpenGLES API传入着色器的只读值
// 本质上一个着色器的任何参数在所有顶点或片元中都应该以统一变量的形式传入
// 在编译时已知的变量应该是常量，不是统一变量。
// 统一变量的命名空间在顶点着色器和片元着色器中是共享的，类似C语言中的static
// 如果在顶点着色器和片元着色器中都声明一个统一变量，那么两个声明必须匹配
// 使用API加载统一变量时，值在顶点和片元着色器中都可用
uniform mat4 ViewProjMatrix;
uniform mat4 viewMatrix;
uniform vec3 lightPosition;
// 统一变量块，实际上是一个缓冲区对象。
// 缓冲区数据可以在多个程序中共享，但只需要设置一次。
layout(std140) uniform TransformBlock
{
    mat4 matViewProj;
    // 单独的统一变量块以及其内部的单独统一变量也可以指定布局限定符
    layout(row_major) mat3 matNormal;
    mat3 matTexGen;
};

// 布局限定符，用于指定支持统一变量块的统一缓冲对象在内存中的布局方式
// shared - 指定多个着色器活着多个程序中统一变量块的内存布局相同
// packed - 指定编译器可以优化统一变量块的内存布局。使用时必须查询偏移位置，且统一变量块无法在顶点／片元着色器或程序间共享。
// std140 - 基于OpenGLES3.0规范的"标准统一变量块布局"中定义的一组标准规则
// row_major - 矩阵在内存中以行优先顺序布局
// column_major - 矩阵在内存中以列优先顺序布局(默认)
layout(shared, column_major) uniform;

// 顶点输入变量/属性变量
// 用于指定顶点着色器每个顶点的输入
uniform mat4 u_matViewProjection;
layout(location = 0) in vec4 a_postion;
layout(location = 1) in vec3 a_color;
// 在顶点说着色器中，v_color被声明为输出变量，那么要在片元着色器中使用，则需要在片元着色器的声明名字相同的in变量
// smooth - 插值限定符，没有插值限定符时，默认插值行为是平滑着色，也就是顶点着色器输出变量在图元中进行先行插值
// flat - 平面着色。在平面着色中，图元的值没有进行插值，而是将其中一个顶点视为驱动顶点，该顶点的值被用于图元的所有片元
smooth out vec3 v_color;


// 不变性声明
// 两个着色器之间等价计算不能保证产生完全相同的结果，可能产生精度丢失的情况，
// 因此需要不变性保证多次计算输出的结果是一致的
invariant gl_Position;
invariant texCoord;
// 也可以用 #pragma 指定所有变量都不变
// #pragma STDGL invariant (all)

// 结构体
struct fogStruct
{
    vec4 color;
    float start;
    float end;
} fogVar;

void main() {
    // 变量
    vec3 myVec3 = vec3(0.0, 1.0, 2.0);
    vec3 temp;
    temp = myVec3.xyz;  // temp = {0.0, 1.0, 2.0}
    temp = myVec3.xxx;  // temp = {0.0, 0.0, 0.0}
    temp = myVec3.zyx;  // temp = {2.0, 1.0, 0.0}
    // 常量
    const float zero = 0.0;
    const float pi = 3.14159;
    const vec4 red = vec4(1.0, 0.0, 0.0, 1.0);

    // 结构体初始化，必须一一对应
    fogVar = fogStruct(vec4(0.0, 1.0, 0.0, 0.0), 0.5, 2.0);
    // 获取结构体变量
    vec4 color = fogVar.color;
    float start = fogVar.start;
    float end = fogVar.end;

    // 数组
    float floatArray[4];
    vec4 vecArray[2];
    float a[4] = float[] (1.0, 2.0, 3.0, 4.0);
    float b[4] = float[4] (1.0, 2.0, 3.0, 4.0);
    vec2 c[2] = vec2[2](vec2(1.0), vec2(1.0));

    // 运算符，大部分表现和C中的一样，乘法可以用在浮点、向量和矩阵之间进行运算
    // 除了 == 和 != 外，比较运算符智能用于标量值，要比较向量可以使用内建函数
    float myFloat;
    vec4 myVec4;
    mat4 myMat4;
    myVec4 = myVec4 * myFloat;
    myVec4 = myVec4 * myVec4;
    myVec4 = myMat4 * myVec4;
    myMat4 = myMat4 * myMat4;
    myMat4 = myMat4 * myFloat;

    // 控制流程语句，跟C语言一样
    if (color.a < 0.25) {
        color *= color.a;
    } else {
        color = myVec4(0.0);
    }

    // 使用统一变量块的例子：
    // 使用了声明的a_postion 和 TransformBlock缓冲区中的matViewProj
    // gl_Position = matViewProj * a_position;

    // 着色器的输入变量a_position 和 a_color 数据由应用程序加载。
    // 本质上应用程序将为每个顶点创建一个顶点数组创建了一个顶点数组，包含了位置和颜色
    gl_Position = u_matViewProjection * a_postion;
    v_color = a_color;


}


// 函数声明
// 限定符
// in - 这个限定符号指定参数按值传递，函数不能修改
// inout - 这个限定符规定变量按照引用传入函数，函数中修改了该值，该变量是会跟着改变的
// out - 表示该变量的值不被传入函数，但是在函数返回时被修改
vec4 myFunc(inout float myFloat,
            out vec4 myVec4,
            mat4 myMat4);

// 定义一个计算基本漫反射光线的函数：
vec4 diffuse(vec3 normal, vec3 light, vec4 backColor) {
    // dot 是内建函数，计算向量的点积， pow函数则是计算幂次
    return baseColor * dot(normal, light);
}
