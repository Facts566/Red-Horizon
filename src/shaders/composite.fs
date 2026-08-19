#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0; // scene
uniform sampler2D texture1; // bloom
uniform vec4 colDiffuse;

uniform float bloomStrength;
uniform float vignetteStrength;

out vec4 finalColor;

void main()
{
    vec3 scene = texture(texture0, fragTexCoord).rgb;
    vec3 bloom = texture(texture1, vec2(fragTexCoord.x, 1.0 - fragTexCoord.y)).rgb;
    vec3 color = scene + bloom * bloomStrength;

    vec2 uv = fragTexCoord - 0.5;
    float d = length(uv);
    float vignette = 1.0 - smoothstep(0.18, 0.75, d);
    color *= mix(1.0, vignette, vignetteStrength);

    finalColor = vec4(color, 1.0);
}