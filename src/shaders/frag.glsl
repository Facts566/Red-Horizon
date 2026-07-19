#version 330
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPosition;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightRange;
uniform float ambientStrength;
uniform vec3 light2Position;
uniform vec3 light2Color;
uniform float light2Range;
out vec4 finalColor;
void main() {
    vec4 texColor = texture(texture0, fragTexCoord);
    if (texColor.a < 0.1) discard;
    vec3 norm = normalize(fragNormal);

    vec3 lightDir = lightPosition - fragPosition;
    float dist = length(lightDir);
    lightDir = normalize(lightDir);
    if (dot(norm, lightDir) < 0.0) norm = -norm;
    float diff = max(dot(norm, lightDir), 0.0);
    float atten = clamp(1.0 - dist / lightRange, 0.0, 1.0);
    atten = atten * atten;
    vec3 ambient = texColor.rgb * ambientStrength;
    vec3 diffuse = texColor.rgb * diff * atten * lightColor;

    vec3 l2dir = light2Position - fragPosition;
    float l2dist = length(l2dir);
    l2dir = normalize(l2dir);
    float l2diff = max(dot(norm, l2dir), 0.0);
    float l2atten = clamp(1.0 - l2dist / light2Range, 0.0, 1.0);
    l2atten = l2atten * l2atten;
    diffuse += texColor.rgb * l2diff * l2atten * light2Color;

    finalColor = vec4(ambient + diffuse, texColor.a) * colDiffuse;
}
