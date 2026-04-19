#version 450

layout(location = 0) in vec4 a_Position;

void main() {
  gl_Position = vec4(a_Position.xyz, 1.0f);
  gl_PointSize = 10.0;
}
