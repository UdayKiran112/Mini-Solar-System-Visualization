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

glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
float yaw = -90.0f; // Horizontal angle, init looking at -Z
float pitch = 0.0f; // Vertical angle
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float fov = 45.0f;

bool leftMousePressed = false;
double lastMouseX, lastMouseY;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            leftMousePressed = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
        else if (action == GLFW_RELEASE)
        {
            leftMousePressed = false;
        }
    }
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (leftMousePressed)
    {
        float xoffset = float(xpos - lastMouseX);
        float yoffset = float(lastMouseY - ypos); // reversed since y-coords go from bottom to top
        lastMouseX = xpos;
        lastMouseY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 15.0f)
        fov = 15.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}

std::vector<float> generateCircleVertices(float radiusX, float radiusY, int segments)
{
    std::vector<float> vertices;
    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * M_PI * i / segments;
        vertices.push_back(radiusX * cos(angle));
        vertices.push_back(radiusY * sin(angle));
    }
    return vertices;
}

std::vector<float> generateCircleVertices(float radius, int segments)
{
    return generateCircleVertices(radius, radius, segments);
}

struct SphereMesh
{
    std::vector<float> vertices; // position (3), normal (3), tex coords (2) = 8 floats per vertex
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;
};

SphereMesh generateSphere(float radius, int sectorCount, int stackCount)
{
    SphereMesh sphere;

    float x, y, z, xy;                           // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius; // normals
    float s, t;                                  // tex coords

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep; // from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);        // r * cos(u)
        y = radius * sinf(stackAngle);         // r * sin(u)

        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep; // from 0 to 2pi

            x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            z = xy * sinf(sectorAngle); // r * cos(u) * sin(v)

            // position
            sphere.vertices.push_back(x);
            sphere.vertices.push_back(y);
            sphere.vertices.push_back(z);

            // normalized normal vector
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            sphere.vertices.push_back(nx);
            sphere.vertices.push_back(ny);
            sphere.vertices.push_back(nz);

            // texture coords
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            sphere.vertices.push_back(s);
            sphere.vertices.push_back(t);
        }
    }

    // indices
    unsigned int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1); // beginning of current stack
        k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                sphere.indices.push_back(k1);
                sphere.indices.push_back(k2);
                sphere.indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1))
            {
                sphere.indices.push_back(k1 + 1);
                sphere.indices.push_back(k2);
                sphere.indices.push_back(k2 + 1);
            }
        }
    }

    // Setup OpenGL buffers and arrays:
    glGenVertexArrays(1, &sphere.VAO);
    glGenBuffers(1, &sphere.VBO);
    glGenBuffers(1, &sphere.EBO);

    glBindVertexArray(sphere.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphere.VBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.vertices.size() * sizeof(float), sphere.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices.size() * sizeof(unsigned int), sphere.indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texcoords attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return sphere;
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

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSwapInterval(1);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Realistic Solar System", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST); // Enable depth test for 3D correct rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader textureShader("shaders/planet.vert", "shaders/planet.frag");
    Shader orbitShader("shaders/orbit.vert", "shaders/orbit.frag");

    // Create circle vertex data with positions and texture coords (for planets)
    std::vector<float> circleVertices = {
        0.0f, 0.0f, 0.5f, 0.5f // center vertex with texture coord center
    };

    const int circleSegments = 100;
    for (int i = 0; i <= circleSegments; ++i)
    {
        float angle = 2.0f * M_PI * i / circleSegments;
        circleVertices.push_back(cos(angle));               // x
        circleVertices.push_back(sin(angle));               // y
        circleVertices.push_back(0.5f + 0.5f * cos(angle)); // tex x
        circleVertices.push_back(0.5f + 0.5f * sin(angle)); // tex y
    }

    GLuint VAO_planet, VBO_planet;
    glGenVertexArrays(1, &VAO_planet);
    glGenBuffers(1, &VBO_planet);

    glBindVertexArray(VAO_planet);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_planet);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0); // positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float))); // tex coords
    glEnableVertexAttribArray(1);

    std::vector<float> orbitVertices = generateCircleVertices(1.0f, 1.0f, circleSegments);

    GLuint VAO_orbit, VBO_orbit;
    glGenVertexArrays(1, &VAO_orbit);
    glGenBuffers(1, &VBO_orbit);
    glBindVertexArray(VAO_orbit);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_orbit);
    glBufferData(GL_ARRAY_BUFFER, orbitVertices.size() * sizeof(float), orbitVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

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

    double previousTime = glfwGetTime();
    int frameCount = 0;

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - previousTime >= 1.0)
        {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            previousTime = currentTime;
        }

        glClearColor(0.02f, 0.02f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear depth buffer too

        float time = (float)glfwGetTime();
        float aspect = (float)SCR_WIDTH / SCR_HEIGHT;

        glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + front, glm::vec3(0, 1, 0));

        // Draw Orbits (using orbitShader)
        orbitShader.use();
        orbitShader.setMat4("projection", glm::value_ptr(projection));
        orbitShader.setMat4("view", glm::value_ptr(view));

        glBindVertexArray(VAO_orbit);
        for (auto &p : planets)
        {
            glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(p.orbitA, p.orbitB, 1.0f));
            orbitShader.setVec3("orbitColor", glm::vec3(0.6f));
            orbitShader.setMat4("model", glm::value_ptr(model));
            glDrawArrays(GL_LINE_LOOP, 0, circleSegments + 1);
        }

        // Draw Sun
        textureShader.use();
        textureShader.setMat4("projection", glm::value_ptr(projection));
        textureShader.setMat4("view", glm::value_ptr(view));
        textureShader.setFloat("time", time);

        glm::mat4 sunModel = glm::scale(glm::mat4(1.0f), glm::vec3(0.15f));
        textureShader.setMat4("model", glm::value_ptr(sunModel));
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        glBindVertexArray(VAO_planet);
        glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(circleVertices.size() / 4));

        // Earth Position
        float earthAngle = time * planets[2].orbitSpeed;
        glm::vec3 earthPos = {
            planets[2].orbitA * cos(earthAngle),
            planets[2].orbitB * sin(earthAngle),
            0.0f};

        glm::mat4 earthModel = glm::translate(glm::mat4(1.0f), earthPos);
        earthModel = glm::rotate(earthModel, time * planets[2].rotationSpeed, glm::vec3(0, 0, 1));
        earthModel = glm::scale(earthModel, glm::vec3(planets[2].radius));
        textureShader.setMat4("model", glm::value_ptr(earthModel));

        glBindTexture(GL_TEXTURE_2D, planets[2].texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(circleVertices.size() / 4));

        // Moon Orbit around Earth
        orbitShader.use();
        glm::mat4 moonOrbitModel = glm::translate(glm::mat4(1.0f), earthPos);
        moonOrbitModel = glm::scale(moonOrbitModel, glm::vec3(planets[3].orbitA, planets[3].orbitB, 1.0f));
        orbitShader.setMat4("model", glm::value_ptr(moonOrbitModel));
        orbitShader.setVec3("orbitColor", glm::vec3(0.8f));
        orbitShader.setMat4("view", glm::value_ptr(view));

        glBindVertexArray(VAO_orbit);
        glDrawArrays(GL_LINE_LOOP, 0, circleSegments + 1);

        // Moon Position
        float moonAngle = time * planets[3].orbitSpeed;
        glm::vec3 moonOffset = {
            planets[3].orbitA * cos(moonAngle),
            planets[3].orbitB * sin(moonAngle),
            0.0f};

        glm::vec3 moonPos = earthPos + moonOffset;
        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), moonPos);
        moonModel = glm::rotate(moonModel, time * planets[3].rotationSpeed, glm::vec3(0, 0, 1));
        moonModel = glm::scale(moonModel, glm::vec3(planets[3].radius));
        textureShader.use();
        textureShader.setMat4("projection", glm::value_ptr(projection));
        textureShader.setMat4("view", glm::value_ptr(view));
        textureShader.setMat4("model", glm::value_ptr(moonModel));

        glBindTexture(GL_TEXTURE_2D, planets[3].texture);
        glBindVertexArray(VAO_planet);
        glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(circleVertices.size() / 4));

        // Draw other planets
        for (size_t i = 0; i < planets.size(); ++i)
        {
            if (i == 2 || i == 3)
                continue; // Already drawn Earth and Moon

            float angle = time * planets[i].orbitSpeed;
            glm::vec3 pos = {
                planets[i].orbitA * cos(angle),
                planets[i].orbitB * sin(angle),
                0.0f};

            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            model = glm::rotate(model, time * planets[i].rotationSpeed, glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(planets[i].radius));

            textureShader.use();
            textureShader.setMat4("projection", glm::value_ptr(projection));
            textureShader.setMat4("view", glm::value_ptr(view));
            textureShader.setMat4("model", glm::value_ptr(model));

            glBindTexture(GL_TEXTURE_2D, planets[i].texture);
            glBindVertexArray(VAO_planet);
            glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)(circleVertices.size() / 4));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO_planet);
    glDeleteBuffers(1, &VBO_planet);
    glDeleteVertexArrays(1, &VAO_orbit);
    glDeleteBuffers(1, &VBO_orbit);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
