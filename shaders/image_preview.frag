#version 450

#define MAX_TEXTURES 10000

layout(location = 0) in vec2 i_UV;
layout(location = 1) flat in int i_TextureIdx;

layout(set = 1, binding = 0) uniform sampler2D textures[MAX_TEXTURES];

layout(location = 0) out vec4 o_FragColor;

void main() {
  o_FragColor = texture(textures[i_TextureIdx], i_UV);
}
