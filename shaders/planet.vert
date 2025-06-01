#version 330 core
layout(location = 0) in vec2 aPos;       // We only have 2D circle vertex positions (x,y)
layout(location = 1) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;

void main() {
    // Convert 2D position to 3D by adding z=0
    vec4 worldPos = model * vec4(aPos.xy, 0.0, 1.0);
    gl_Position = projection * view * worldPos;
    TexCoord = aTexCoord;
}
