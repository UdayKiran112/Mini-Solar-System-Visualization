#ifndef SPHERE_H
#define SPHERE_H

#include <../external/glad/include/glad/glad.h>
#include <vector>

class Sphere
{
private:
    std::vector<float> sphere_vertices;
    std::vector<int> sphere_indices;
    GLuint VBO, VAO, EBO;
    float radius;
    int sectorCount;
    int stackCount;

    void generateVertices();
    void generateIndices();
    void setupBuffers();

public:
    Sphere(float r, int sectors, int stacks);
    ~Sphere();
    void Draw();
};

#endif
