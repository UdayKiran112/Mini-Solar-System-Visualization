#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform float time;

void main() {
    // Create a pulsating sun effect
    vec4 sunColor = texture(texture1, TexCoord);

    // Pulsing glow factor between 0.8 and 1.2
    float pulse = 0.8 + 0.4 * sin(time * 3.0);

    // Amplify brightness with pulse
    vec3 glow = sunColor.rgb * pulse;

    FragColor = vec4(glow, sunColor.a);
}
