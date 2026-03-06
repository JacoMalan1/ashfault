#version 450

layout(location = 0) in vec3 i_Normal;
layout(location = 1) in vec3 i_FragPos;

layout(location = 0) out vec4 o_FragColor;

void main() {
  vec3 light = vec3(0.0, 0.5, -2.0);
  vec3 dir = normalize(light - i_FragPos);
  float diff = max(dot(i_Normal, dir), 0.0);
  vec3 col = diff * vec3(1.0, 1.0, 1.0);
  o_FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
