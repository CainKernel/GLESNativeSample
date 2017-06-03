//
// Created by cain on 2017/5/29.
//

#include <GLES3/gl3.h>

typedef int vertex_t;

/**
 * 创建和绑定顶点缓冲区对象
 * @param vertexBuffer
 * @param indices
 * @param numVertices
 * @param numIndices
 * @param vboIds
 */
void initVertexBufferObjects(vertex_t *vertexBuffer,
                             GLushort *indices,
                             GLuint numVertices,
                             GLuint numIndices,
                             GLuint *vboIds) {

    // 获取vboIds中两个未用的缓冲区对象名称。然后vboIds返回的未使用的缓冲区对象名称用于创建一个数组缓冲区对象
    // 和一个元素数组缓冲区对象。数组缓冲区对象用于保存一个或多个图元的顶点属性数据。
    glGenBuffers(2, vboIds);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(vertex_t),
                 vertexBuffer, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLushort),
                 indices, GL_STATIC_DRAW);
}