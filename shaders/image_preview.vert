#version 450

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_UV;

layout(push_constant) uniform TexturePreviewData {
  int textureIdx;
} data;

layout(location = 0) out vec2 o_UV;
layout(location = 1) out int o_TextureIdx;

void main() {
  gl_Position = vec4(a_Position, 0.0, 1.0);

  o_UV = a_UV;
  o_TextureIdx = data.textureIdx;
}
