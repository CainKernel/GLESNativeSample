//
// Created by cain on 2017/5/27.
//


//四个顶点属性（位置、法线和两个纹理坐标）组成缓冲区布局如下：
//     位置        法线      纹理0   纹理1
// |x | y | z | x | y | z | s | t | s | t |

// 结构数组
#include <cstdlib>
#include <GLES3/gl3.h>

#define VERTEX_POS_SIZE 3
#define VERTEX_NORMAL_SIZE 3
#define VERTEX_TEXCOORD0_SIZE 2
#define VERTEX_TEXCOORD1_SIZE 2

#define VERTEX_POS_INDX 0
#define VERTEX_NORMAL_INDX 1
#define VERTEX_TEXCOORD0_INDX 2
#define VERTEX_TEXCOORD1_INDX 3

#define VERTEX_POS_OFFSET 0
#define VERTEX_NORMAL_OFFSET 3
#define VERTEX_TEXCOORD0_OFFSET 6
#define VERTEX_TEXCOORD1_OFFSET 8

#define VERTEX_ATTRIB_SIZE (VERTEX_POS_SIZE + VERTEX_NORMAL_SIZE + VERTEX_TEXCOORD0_SIZE + VERTEX_TEXCOORD1_SIZE)

// 结构数组例子，使用了缓冲区，这种分配方法最高效。因为每个顶点的属性数据可以顺序读取，这是最高效的内存访问模式
void structSample(int numVertices) {

    float *p = (float *) malloc(sizeof(float) * VERTEX_ATTRIB_SIZE * numVertices);

    // 位置属性
    glVertexAttribPointer(VERTEX_POS_INDX, VERTEX_POS_SIZE, GL_FLOAT, GL_FALSE, VERTEX_ATTRIB_SIZE *
            sizeof(float), (p + VERTEX_POS_OFFSET));


    // 法线属性
    glVertexAttribPointer(VERTEX_NORMAL_INDX, VERTEX_NORMAL_SIZE, GL_FLOAT, GL_FALSE, VERTEX_ATTRIB_SIZE *
            sizeof(float), (p + VERTEX_NORMAL_OFFSET));

    // 纹理0 属性
    glVertexAttribPointer(VERTEX_TEXCOORD0_INDX, VERTEX_TEXCOORD0_SIZE, GL_FLOAT, GL_FALSE, VERTEX_ATTRIB_SIZE *
            sizeof(float), (p + VERTEX_TEXCOORD0_OFFSET));

    // 纹理1 属性
    glVertexAttribPointer(VERTEX_TEXCOORD1_INDX, VERTEX_TEXCOORD1_SIZE, GL_FLOAT, GL_FALSE, VERTEX_ATTRIB_SIZE *
            sizeof(float), (p + VERTEX_TEXCOORD1_OFFSET));
}

// 数组结构，位置、法线、纹理坐标都保存在单独的缓冲区中
void arraySample(int numVertices) {
    float *position = (float *)malloc(numVertices * VERTEX_POS_SIZE * sizeof(float));
    float *normal = (float *)malloc(numVertices * VERTEX_NORMAL_SIZE * sizeof(float));
    float *texcoord0 = (float *)malloc(numVertices * VERTEX_TEXCOORD0_SIZE * sizeof(float));
    float *texcoord1 =  (float *)malloc(numVertices * VERTEX_TEXCOORD1_SIZE * sizeof(float));

    glVertexAttribPointer(VERTEX_POS_INDX, VERTEX_POS_SIZE, GL_FLOAT, GL_FALSE, VERTEX_POS_SIZE *
            sizeof(float), position);
    glVertexAttribPointer(VERTEX_NORMAL_INDX, VERTEX_NORMAL_SIZE, GL_FLOAT, GL_FALSE, VERTEX_NORMAL_SIZE *
            sizeof(float), normal);
    glVertexAttribPointer(VERTEX_TEXCOORD0_INDX, VERTEX_TEXCOORD0_SIZE, GL_FLOAT, GL_FALSE, VERTEX_TEXCOORD0_SIZE *
            sizeof(float), texcoord0);
    glVertexAttribPointer(VERTEX_TEXCOORD1_INDX, VERTEX_TEXCOORD1_SIZE, GL_FLOAT, GL_FALSE, VERTEX_TEXCOORD1_SIZE *
            sizeof(float), texcoord1);
}
