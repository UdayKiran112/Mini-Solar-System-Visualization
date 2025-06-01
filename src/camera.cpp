#include "camera.h"

// Constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, GLfloat yaw, GLfloat pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM),
      FreeCam(false)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// Constructor with scalar values
Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ,
               GLfloat upX, GLfloat upY, GLfloat upZ,
               GLfloat yaw, GLfloat pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM),
      FreeCam(false)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// View matrix using LookAt
glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

// Keyboard input
void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
{
    GLfloat velocity = MovementSpeed * deltaTime;

    if (FreeCam)
    {
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }
    else
    {
        if (direction == FORWARD)
            Position += glm::vec3(0.0f, 0.0f, 1.0f) * velocity;
        if (direction == BACKWARD)
            Position -= glm::vec3(0.0f, 0.0f, 1.0f) * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == SCROLL_FORWARD && Position.y > 70.0f)
            Position += Front * (10000.0f * deltaTime);
        if (direction == SCROLL_BACKWARD)
            Position -= Front * (15000.0f * deltaTime);
    }
}

// Mouse movement input
void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    if (FreeCam)
    {
        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }
    }

    updateCameraVectors();
}

// Scroll input
void Camera::ProcessMouseScroll(GLfloat yoffset)
{
    if (Zoom >= 1.0f && Zoom <= 45.0f)
        Zoom -= yoffset;
    if (Zoom <= 1.0f)
        Zoom = 1.0f;
    if (Zoom >= 45.0f)
        Zoom = 45.0f;
}

// Recalculate the camera vectors from the updated Euler angles
void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
