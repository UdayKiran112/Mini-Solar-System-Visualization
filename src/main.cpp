#include <../external/glad/include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// stb_easy_font.h must be included for drawText if you want labels (not included here)

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

std::vector<float> generateCircleVertices(float radius, int segments)
{
    std::vector<float> vertices;
    vertices.push_back(0.0f); // center point
    vertices.push_back(0.0f);
    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(radius * cos(angle));
        vertices.push_back(radius * sin(angle));
    }
    return vertices;
}

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mini Solar System", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);

    // Load shaders
    Shader sunShader("shaders/sun.vert", "shaders/sun.frag");
    Shader planetShader("shaders/planet.vert", "shaders/planet.frag");

    // Sun geometry
    auto sunVertices = generateCircleVertices(0.2f, 100);
    unsigned int sunVAO, sunVBO;
    glGenVertexArrays(1, &sunVAO);
    glGenBuffers(1, &sunVBO);

    glBindVertexArray(sunVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
    glBufferData(GL_ARRAY_BUFFER, sunVertices.size() * sizeof(float), sunVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Planet geometry
    auto planetVertices = generateCircleVertices(0.05f, 50);
    unsigned int planetVAO, planetVBO;
    glGenVertexArrays(1, &planetVAO);
    glGenBuffers(1, &planetVBO);

    glBindVertexArray(planetVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planetVBO);
    glBufferData(GL_ARRAY_BUFFER, planetVertices.size() * sizeof(float), planetVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Moon geometry
    auto moonVertices = generateCircleVertices(0.02f, 30);
    unsigned int moonVAO, moonVBO;
    glGenVertexArrays(1, &moonVAO);
    glGenBuffers(1, &moonVBO);

    glBindVertexArray(moonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, moonVBO);
    glBufferData(GL_ARRAY_BUFFER, moonVertices.size() * sizeof(float), moonVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        float time = glfwGetTime();

        float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
        glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f);

        // Draw Sun
        sunShader.use();
        sunShader.setMat4("projection", &projection[0][0]);
        glm::mat4 sunModel = glm::mat4(1.0f);
        sunShader.setMat4("model", &sunModel[0][0]);
        glBindVertexArray(sunVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sunVertices.size() / 2);

        // Draw Earth (planet)
        planetShader.use();
        planetShader.setVec3("color", glm::vec3(0.1f, 0.6f, 1.0f));
        planetShader.setMat4("projection", &projection[0][0]);

        float planetOrbitRadius = 0.6f;
        float planetAngle = time;
        glm::vec3 planetPos(cos(planetAngle) * planetOrbitRadius, sin(planetAngle) * planetOrbitRadius, 0.0f);

        glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), planetPos);
        planetModel = glm::rotate(planetModel, time * 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        planetShader.setMat4("model", &planetModel[0][0]);

        glBindVertexArray(planetVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, planetVertices.size() / 2);

        // Draw Moon
        planetShader.setVec3("color", glm::vec3(0.8f, 0.8f, 0.8f));
        float moonOrbitRadius = 0.15f;
        float moonAngle = time * 4.0f;
        glm::vec3 moonOffset(cos(moonAngle) * moonOrbitRadius, sin(moonAngle) * moonOrbitRadius, 0.0f);

        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), planetPos + moonOffset);
        moonModel = glm::rotate(moonModel, time * 3.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        planetShader.setMat4("model", &moonModel[0][0]);

        glBindVertexArray(moonVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, moonVertices.size() / 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &sunVAO);
    glDeleteBuffers(1, &sunVBO);
    glDeleteVertexArrays(1, &planetVAO);
    glDeleteBuffers(1, &planetVBO);
    glDeleteVertexArrays(1, &moonVAO);
    glDeleteBuffers(1, &moonVBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
