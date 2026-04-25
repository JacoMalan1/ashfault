#version 450

layout(location = 0) in vec4 a_Tangent;
layout(location = 1) in vec3 a_Position;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec2 a_UV;

layout(location = 0) out vec3 o_Normal;
layout(location = 1) out vec3 o_FragPos;
layout(location = 2) out vec2 o_UV;
layout(location = 3) out mat3 o_TBN;

layout(push_constant) uniform PushConstants {
  mat4 proj_mat;
  mat4 view_mat;
  mat4 model;
  int albedo_tex_index;
  int normal_tex_index;
  int roughness_tex_index;
  int metallic_tex_index;
  float diffuse;
  float specular;
} pc;

void main() {
  gl_Position = pc.proj_mat * pc.view_mat * pc.model * vec4(a_Position, 1.0);
  o_Normal = normalize(mat3(transpose(inverse(pc.view_mat * pc.model))) * a_Normal);
  o_FragPos = (pc.view_mat * pc.model * vec4(a_Position, 1.0)).xyz;

  if (pc.normal_tex_index != 0) {
    vec3 tangent = normalize(mat3(pc.view_mat * pc.model) * a_Tangent.xyz);
    tangent = normalize(tangent - o_Normal * dot(o_Normal, tangent));
    vec3 bitangent = cross(o_Normal, tangent) * a_Tangent.w;
    o_TBN = mat3(tangent, bitangent, o_Normal);
  } else {
    o_TBN = mat3(1);
  }

  o_UV = a_UV;
}
