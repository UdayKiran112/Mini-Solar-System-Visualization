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

const unsigned int SCR_WIDTH_DEFAULT = 1280;
const unsigned int SCR_HEIGHT_DEFAULT = 720;

unsigned int SCR_WIDTH = SCR_WIDTH_DEFAULT;
unsigned int SCR_HEIGHT = SCR_HEIGHT_DEFAULT;

// Generate circle vertices for elliptical or circular orbit (2D)
std::vector<float> generateCircleVertices(float radiusX, float radiusY, int segments)
{
    std::vector<float> vertices;
    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * M_PI * i / segments;
        float x = radiusX * cos(angle);
        float y = radiusY * sin(angle);
        vertices.push_back(x);
        vertices.push_back(y);
    }
    return vertices;
}

// Overload for circle (equal radii)
std::vector<float> generateCircleVertices(float radius, int segments)
{
    return generateCircleVertices(radius, radius, segments);
}

struct Planet
{
    float radius;
    float orbitA, orbitB;
    float orbitSpeed;
    float rotationSpeed;
    GLuint texture;
    glm::vec3 color; // Currently unused, can be used for color tinting if needed
};

// Callback to adjust viewport on window resize
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

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

    // Create GLFW window
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Realistic Solar System", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL function pointers with GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Enable alpha blending for transparency support
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load shaders
    Shader textureShader("shaders/planet.vert", "shaders/planet.frag");
    Shader orbitShader("shaders/orbit.vert", "shaders/orbit.frag");

    // Generate circle vertices for textured planet (with position + texcoords)
    std::vector<float> circleVertices;
    // Center vertex (pos: 0,0 texcoord: 0.5,0.5)
    circleVertices.push_back(0.0f);
    circleVertices.push_back(0.0f);
    circleVertices.push_back(0.5f);
    circleVertices.push_back(0.5f);

    const int circleSegments = 100;
    for (int i = 0; i <= circleSegments; ++i)
    {
        float angle = 2.0f * M_PI * i / circleSegments;
        float x = cos(angle);
        float y = sin(angle);
        circleVertices.push_back(x);
        circleVertices.push_back(y);
        // Map texture coordinates (0.5 + 0.5*cos, 0.5 + 0.5*sin)
        circleVertices.push_back(0.5f + 0.5f * cos(angle));
        circleVertices.push_back(0.5f + 0.5f * sin(angle));
    }

    // Setup VAO/VBO for planet circle
    GLuint VAO_planet, VBO_planet;
    glGenVertexArrays(1, &VAO_planet);
    glGenBuffers(1, &VBO_planet);

    glBindVertexArray(VAO_planet);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_planet);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);

    // position attribute (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texcoord attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Generate elliptical orbit vertices (just positions)
    std::vector<float> orbitVertices = generateCircleVertices(1.0f, 1.0f, circleSegments);

    // Setup VAO/VBO for orbit circle
    GLuint VAO_orbit, VBO_orbit;
    glGenVertexArrays(1, &VAO_orbit);
    glGenBuffers(1, &VBO_orbit);

    glBindVertexArray(VAO_orbit);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_orbit);
    glBufferData(GL_ARRAY_BUFFER, orbitVertices.size() * sizeof(float), orbitVertices.data(), GL_STATIC_DRAW);

    // Only position attribute for orbit (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Load planet textures
    std::vector<Planet> planets = {
        {0.05f, 0.25f, 0.20f, 1.60f, 4.0f, loadTexture("textures/2k_mercury.jpg"), {}},
        {0.06f, 0.35f, 0.30f, 1.20f, 3.8f, loadTexture("textures/2k_venus_surface.jpg"), {}},
        {0.07f, 0.45f, 0.40f, 1.00f, 3.0f, loadTexture("textures/2k_earth_daymap.jpg"), {}},
        {0.025f, 0.10f, 0.07f, 5.0f, 3.0f, loadTexture("textures/2k_moon.jpg"), {}},
        {0.06f, 0.60f, 0.50f, 0.80f, 2.8f, loadTexture("textures/2k_mars.jpg"), {}},
        {0.12f, 0.80f, 0.70f, 0.60f, 2.6f, loadTexture("textures/2k_jupiter.jpg"), {}},
        {0.10f, 1.00f, 0.90f, 0.45f, 2.4f, loadTexture("textures/2k_saturn.jpg"), {}},
        {0.09f, 1.15f, 1.05f, 0.35f, 2.2f, loadTexture("textures/2k_uranus.jpg"), {}},
        {0.08f, 1.30f, 1.20f, 0.25f, 2.0f, loadTexture("textures/2k_neptune.jpg"), {}},
    };

    GLuint sunTexture = loadTexture("textures/2k_sun.jpg");

    // FPS variables
    double previousTime = glfwGetTime();
    int frameCount = 0;

    while (!glfwWindowShouldClose(window))
    {
        // FPS calculation every second
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - previousTime >= 1.0)
        {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            previousTime = currentTime;
        }

        // Clear screen with dark background
        glClearColor(0.02f, 0.02f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float time = (float)glfwGetTime();
        float aspect = static_cast<float>(SCR_WIDTH) / SCR_HEIGHT;

        // Orthographic projection with dynamic aspect ratio
        glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f);

        // --- Draw Orbits ---
        orbitShader.use();
        orbitShader.setMat4("projection", glm::value_ptr(projection));
        glBindVertexArray(VAO_orbit);

        for (auto &p : planets)
        {
            glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(p.orbitA, p.orbitB, 1.0f));
            orbitShader.setMat4("model", glm::value_ptr(model));
            glDrawArrays(GL_LINE_LOOP, 0, circleSegments + 1);
        }

        // --- Draw Sun ---
        textureShader.use();
        textureShader.setMat4("projection", glm::value_ptr(projection));
        glm::mat4 sunModel = glm::scale(glm::mat4(1.0f), glm::vec3(0.15f));
        textureShader.setMat4("model", glm::value_ptr(sunModel));
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        glBindVertexArray(VAO_planet);
        glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(circleVertices.size() / 4));

        // --- Draw Planets ---
        for (auto &p : planets)
        {
            float angle = time * p.orbitSpeed;
            float x = p.orbitA * cos(angle);
            float y = p.orbitB * sin(angle);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::rotate(model, time * p.rotationSpeed, glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(p.radius));

            textureShader.setMat4("model", glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, p.texture);
            glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(circleVertices.size() / 4));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO_planet);
    glDeleteBuffers(1, &VBO_planet);
    glDeleteVertexArrays(1, &VAO_orbit);
    glDeleteBuffers(1, &VBO_orbit);

    glfwTerminate();
    return 0;
}
