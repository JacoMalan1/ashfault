#version 450

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_UV;

layout(location = 0) out vec2 o_UV;

void main() {
  gl_Position = vec4(a_Position, 0.0, 1.0);

  o_UV = a_UV;
}
