#version 330 core
in vec2 texPos;
in vec3 pos;
in vec3 projPos;
out vec4 FragColor;

uniform sampler2D ourTexture;
//uniform bool isDark;
uniform float interpolation; //between 0 and 1 -> 1 = day, 0 = night

void main() {
    //FragColor = (1 - 0.4 * pos.z) * vec4(0, 1, 0, 1) + (0.4 * pos.z * vec4(0.2, 0.3, 0.3, 1));
    //FragColor = vec4(0, 1, 0, 1);
    /*if (isDark) { //night time lighting
        FragColor = 3 * texture(ourTexture, texPos) / (projPos.z + 1.1);
    }
    else { //day time (no lighting) lighting
        FragColor = texture(ourTexture, texPos);
    }*/
    //interpolate between day and night
    vec4 sampledTexture = texture(ourTexture, texPos);
    FragColor = interpolation * sampledTexture + (1 - interpolation) * (3 * sampledTexture * (1 / (projPos.z + 2)));

    if ((abs(pos.x) < 0.005 * abs(pos.z)|| abs(pos.y) < 0.005 * abs(pos.z)) && (abs(pos.x) < 0.05 * abs(pos.z) && abs(pos.y) < 0.05 * abs(pos.z))) { //crosshair
        FragColor = vec4(vec3(1, 1, 1) - FragColor.xyz, 1);
    }
    //FragColor = texture(ourTexture, texPos) / (log2(log2(pos.z + 2)) + 1);
}