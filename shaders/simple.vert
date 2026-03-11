#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(push_constant) uniform PushConstants {
  mat4 model;
} pc;

layout(binding = 0) uniform UniformBufferObject {
  mat4 projection;
  mat4 view;
} camera;

void main() {
  gl_Position = camera.projection * camera.view * pc.model * vec4(a_Position, 1);
}
