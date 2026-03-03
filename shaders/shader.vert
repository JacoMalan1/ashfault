#version 450

layout(location = 0) in vec3 a_Position;
layout(binding = 0) uniform UBO {
  mat4 proj_mat;
} ubo;

void main() {
  gl_Position = ubo.proj_mat * vec4(a_Position, 1.0);
}
