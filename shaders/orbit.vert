#version 330 core

layout(location = 0) in vec2 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vec4 worldPos = model * vec4(aPos, 0.0, 1.0);
    gl_Position = projection * view * worldPos;
}
