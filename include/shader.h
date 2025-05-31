#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <../external/glad/include/glad/glad.h>
#include <glm/glm.hpp>

class Shader
{
public:
    GLuint ID;

    Shader(const char *vertexPath, const char *fragmentPath);
    void use() const;

    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const float *mat) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type);
};

#endif
