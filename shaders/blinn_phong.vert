#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

layout(location = 0) out vec3 o_Normal;
layout(location = 1) out vec3 o_FragPos;

layout(push_constant) uniform PushConstants {
  mat4 model;
} pc;

layout(binding = 0) uniform UBO {
  mat4 proj_mat;
  mat4 view_mat;
} ubo;

void main() {
  gl_Position = ubo.proj_mat * ubo.view_mat * pc.model * vec4(a_Position, 1.0);
  o_Normal = mat3(transpose(inverse(pc.model))) * a_Normal;
  o_FragPos = (pc.model * vec4(a_Position, 1.0)).xyz;
}
