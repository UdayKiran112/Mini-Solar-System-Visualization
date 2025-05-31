#version 330 core
out vec4 FragColor;
uniform float time;

void main()
{
    float pulse = 0.5 + 0.5 * sin(time * 2.0);
    FragColor = vec4(pulse, pulse * 0.6, 0.0, 1.0); // orange pulsating sun
}
