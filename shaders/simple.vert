#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(push_constant) uniform PushConstants {
  mat4 projection;
  mat4 view;
  mat4 model;
} pc;

void main() {
  gl_Position = pc.projection * pc.view * pc.model * vec4(a_Position, 1);
}
