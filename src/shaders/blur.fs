#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 texelSize;
uniform vec2 blurDirection;

out vec4 finalColor;

void main()
{
    vec2 dir = blurDirection * texelSize;

    vec3 color = texture(texture0, fragTexCoord).rgb * 0.2270270270;
    color += texture(texture0, fragTexCoord + dir * 1.0).rgb * 0.1945945946;
    color += texture(texture0, fragTexCoord - dir * 1.0).rgb * 0.1945945946;
    color += texture(texture0, fragTexCoord + dir * 2.0).rgb * 0.1216216216;
    color += texture(texture0, fragTexCoord - dir * 2.0).rgb * 0.1216216216;
    color += texture(texture0, fragTexCoord + dir * 3.0).rgb * 0.0540540541;
    color += texture(texture0, fragTexCoord - dir * 3.0).rgb * 0.0540540541;

    finalColor = vec4(color, texture(texture0, fragTexCoord).a);
}