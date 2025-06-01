#ifndef CAMERA_H
#define CAMERA_H

// Standard and GL Includes
#include <vector>
#include <../external/glad/include/glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Camera movement options
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    SCROLL_FORWARD,
    SCROLL_BACKWARD
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 400.0f;
const GLfloat SENSITIVITY = 0.1f;
const GLfloat ZOOM = 45.0f;

class Camera
{
public:
    // Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::vec3 LookAtPos;

    // Euler angles
    GLfloat Yaw;
    GLfloat Pitch;

    // Options
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;
    GLfloat Zoom;
    bool FreeCam;

    // Constructors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           GLfloat yaw = YAW,
           GLfloat pitch = PITCH);

    Camera(GLfloat posX, GLfloat posY, GLfloat posZ,
           GLfloat upX, GLfloat upY, GLfloat upZ,
           GLfloat yaw, GLfloat pitch);

    // Get the view matrix
    glm::mat4 GetViewMatrix();

    // Input handlers
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime);
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(GLfloat yoffset);

private:
    // Recalculate camera vectors
    void updateCameraVectors();
};

#endif // CAMERA_H
