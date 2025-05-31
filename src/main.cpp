#include <../external/glad/include/glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

std::vector<float> generateCircleVertices(float radius, int segments)
{
    std::vector<float> vertices;
    // Center vertex: position + tex coords
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.5f);
    vertices.push_back(0.5f);

    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.5f + 0.5f * cos(angle));
        vertices.push_back(0.5f + 0.5f * sin(angle));
    }
    return vertices;
}

struct Planet
{
    float radius;
    float orbitA, orbitB;
    float orbitSpeed;
    float rotationSpeed;
    GLuint texture;
    glm::vec3 color;
};

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Realistic Solar System", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Setup viewport and blending
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load shader program
    Shader textureShader("shaders/planet.vert", "shaders/planet.frag");

    // Generate circle vertices for drawing planets
    auto circleVertices = generateCircleVertices(1.0f, 100);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);

    // Positions (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // TexCoords (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Load planet textures
    std::vector<Planet> planets = {
        {0.05f, 0.25f, 0.20f, 1.60f, 4.0f, loadTexture("textures/2k_mercury.jpg"), {}},
        {0.06f, 0.35f, 0.30f, 1.20f, 3.8f, loadTexture("textures/2k_venus_surface.jpg"), {}},
        {0.07f, 0.45f, 0.40f, 1.00f, 3.0f, loadTexture("textures/2k_earth_daymap.jpg"), {}},
        {0.025f, 0.10f, 0.07f, 5.0f, 3.0f, loadTexture("textures/2k_moon.jpg"), {}}, // Moon (relative to Earth)
        {0.06f, 0.60f, 0.50f, 0.80f, 2.8f, loadTexture("textures/2k_mars.jpg"), {}},
        {0.12f, 0.80f, 0.70f, 0.60f, 2.6f, loadTexture("textures/2k_jupiter.jpg"), {}},
        {0.10f, 1.00f, 0.90f, 0.45f, 2.4f, loadTexture("textures/2k_saturn.jpg"), {}},
        {0.09f, 1.15f, 1.05f, 0.35f, 2.2f, loadTexture("textures/2k_uranus.jpg"), {}},
        {0.08f, 1.30f, 1.20f, 0.25f, 2.0f, loadTexture("textures/2k_neptune.jpg"), {}},
    };

    GLuint sunTexture = loadTexture("textures/2k_sun.jpg");

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.02f, 0.02f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float time = glfwGetTime();
        float aspect = static_cast<float>(SCR_WIDTH) / SCR_HEIGHT;
        glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f);

        textureShader.use();
        textureShader.setMat4("projection", glm::value_ptr(projection));

        // Draw Sun
        glm::mat4 sunModel = glm::scale(glm::mat4(1.0f), glm::vec3(0.15f));
        textureShader.setMat4("model", glm::value_ptr(sunModel));
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertices.size() / 4);

        // Draw planets
        for (size_t i = 0; i < planets.size(); ++i)
        {
            Planet &p = planets[i];

            glm::mat4 model(1.0f);

            if (i == 3)
            {
                // Moon orbiting Earth
                Planet &earth = planets[2];
                float earthX = earth.orbitA * cos(time * earth.orbitSpeed);
                float earthY = earth.orbitB * sin(time * earth.orbitSpeed);
                float moonX = p.orbitA * cos(time * p.orbitSpeed);
                float moonY = p.orbitB * sin(time * p.orbitSpeed);
                model = glm::translate(model, glm::vec3(earthX + moonX, earthY + moonY, 0.0f));
            }
            else
            {
                float x = p.orbitA * cos(time * p.orbitSpeed);
                float y = p.orbitB * sin(time * p.orbitSpeed);
                model = glm::translate(model, glm::vec3(x, y, 0.0f));
            }

            model = glm::rotate(model, time * p.rotationSpeed, glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(p.radius));

            textureShader.setMat4("model", glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, p.texture);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertices.size() / 4);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
