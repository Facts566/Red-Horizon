#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matView;
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragPosition;
void main() {
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize((matModel * vec4(vertexNormal, 0.0)).xyz);
    fragPosition = (matModel * vec4(vertexPosition, 1.0)).xyz;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
