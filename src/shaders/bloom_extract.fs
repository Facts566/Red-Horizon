#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float bloomThreshold;

out vec4 finalColor;

void main()
{
    vec3 color = texture(texture0, fragTexCoord).rgb;
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float amount = smoothstep(bloomThreshold, bloomThreshold + 0.25, luma);
    finalColor = vec4(color * amount, 1.0);
}