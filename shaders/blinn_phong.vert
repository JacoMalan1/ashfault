#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

layout(location = 0) out vec3 o_Normal;
layout(location = 1) out vec3 o_FragPos;
layout(location = 2) out mat4 o_ViewMat;

layout(push_constant) uniform PushConstants {
  mat4 proj_mat;
  mat4 view_mat;
  mat4 model;
} pc;

void main() {
  gl_Position = pc.proj_mat * pc.view_mat * pc.model * vec4(a_Position, 1.0);
  o_Normal = normalize(mat3(transpose(inverse(pc.model))) * a_Normal);
  o_FragPos = (pc.view_mat * pc.model * vec4(a_Position, 1.0)).xyz;
  o_ViewMat = pc.view_mat;
}
