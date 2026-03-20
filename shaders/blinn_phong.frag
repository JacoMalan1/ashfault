#version 450

layout(location = 0) in vec3 i_Normal;
layout(location = 1) in vec3 i_FragPos;

layout(location = 0) out vec4 o_FragColor;

void main() {
  vec3 lightColor = vec3(1);

  vec3 col = vec3(1);
  float ambientStrength = 0.2;
  vec3 ambient = lightColor * ambientStrength;

  vec3 lightPos = vec3(0, 0, 2);
  vec3 lightDir = normalize(lightPos - i_FragPos);
  float diff = max(dot(i_Normal, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  vec3 result = (ambient + diffuse) * col;
  o_FragColor = vec4(result, 1.0);
}
