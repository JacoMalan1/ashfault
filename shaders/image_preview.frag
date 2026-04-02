#version 450

#define MAX_TEXTURES 10000

layout(location = 0) in vec2 i_UV;

layout(push_constant) uniform TexturePreviewData {
  int textureIdx;
} data;

layout(set = 1, binding = 0) uniform sampler2D textures[MAX_TEXTURES];

layout(location = 0) out vec4 o_FragColor;

void main() {
  o_FragColor = texture(textures[data.textureIdx], i_UV);
}
