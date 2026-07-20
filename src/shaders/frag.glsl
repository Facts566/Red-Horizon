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
uniform vec3 lampPos[4];
uniform vec3 lampColor[4];
uniform float lampRange[4];
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

    for (int i = 0; i < 4; i++) {
        vec3 ldir = lampPos[i] - fragPosition;
        float ldist = length(ldir);
        ldir = normalize(ldir);
        float ldiff = max(dot(norm, ldir), 0.0);
        float latten = clamp(1.0 - ldist / lampRange[i], 0.0, 1.0);
        latten = latten * latten;
        diffuse += texColor.rgb * ldiff * latten * lampColor[i];
    }

    finalColor = vec4(ambient + diffuse, texColor.a) * colDiffuse;
}
