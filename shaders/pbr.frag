#version 450
#extension GL_EXT_nonuniform_qualifier : enable

const float PI = 3.14159265359;
const float EPSILON = 0.00001;
const vec3 Fdielectric = vec3(0.04);

#define MAX_LIGHTS 128
#define MAX_TEXTURES 10000

layout(location = 0) in vec3 i_Normal;
layout(location = 1) in vec3 i_FragPos;
layout(location = 2) in vec2 i_UV;
layout(location = 3) in mat3 i_TBN;

layout(location = 0) out vec4 o_FragColor;

layout(push_constant) uniform PushConstants {
  mat4 proj_mat;
  mat4 view_mat;
  mat4 model;

  int albedo_tex_index;
  int normal_tex_index;
  int roughness_tex_index;
  int metallic_tex_index;

  vec3 camera_pos;

  float roughness;
  float metallic;
} pc;

struct Light {
  vec4 position; // w=0 directional, w=1 point
  vec4 direction;
  vec4 color; // rgb + intensity
};

layout(std140, set = 0, binding = 0) uniform Lights {
  Light lights[MAX_LIGHTS];
  int lightCount;
} lights;

layout(set = 1, binding = 0) uniform sampler2D textures[MAX_TEXTURES];

float ndf_ggxtr(vec3 normal, vec3 halfway, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(normal, halfway), 0.0);
  float NdotH2 = NdotH * NdotH;

  float denom = (NdotH * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;
  return a2 / denom;
}

float geometry_schlick_ggx(float NdotV, float k) {
  return NdotV / (NdotV * (1.0 - k) + k);
}

float geometry_smith(vec3 normal, vec3 viewDir, vec3 lightDir, float k) {
  float NdotV = max(dot(normal, viewDir), 0.001);
  float NdotL = max(dot(normal, lightDir), 0.001);
  float ggx1 = geometry_schlick_ggx(NdotV, k);
  float ggx2 = geometry_schlick_ggx(NdotL, k);

  return ggx1 * ggx2;
}

vec3 calculate_light(Light light, vec3 viewDir, vec3 normal, vec3 albedo, float roughness, float metallic) {
  vec3 radiance = vec3(1.0);
  vec3 lightDir = vec3(0.0);
  if (light.position.w == 1.0) {
    vec3 lightPos = (pc.view_mat * vec4(light.position.xyz, 1.0f)).xyz;
    lightDir = normalize(lightPos - i_FragPos);
    float distance = length(lightPos - i_FragPos);
    float attenuation = 1.0 / (distance * distance);
    radiance = light.color.rgb * light.color.a * attenuation;
  } else {
    lightDir = normalize(-(pc.view_mat * vec4(light.direction.xyz, 0.0)).xyz);
    radiance = light.color.rgb * light.color.a;
  }
  float cosTheta = dot(lightDir, normal);
  vec3 halfway = normalize(lightDir + viewDir);

  float D = ndf_ggxtr(normal, halfway, roughness);
  float k = (roughness + 1.0);
  k = (k * k) / 8.0;
  float G = geometry_smith(normal, viewDir, lightDir, k);

  vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
  vec3 F = F0 + (1.0 - F0) * pow(clamp(1.0 - max(dot(halfway, viewDir), 0.0), 0.0, 1.0), 5.0);

  vec3 DGF = D * G * F;

  float w0dotN = max(dot(viewDir, normal), 0.001);
  vec3 cookTorrance = DGF / max(4.0 * w0dotN * cosTheta, EPSILON);
  vec3 kd = vec3(1.0) - F;
  kd *= 1.0 - metallic;

  return (kd * albedo / PI + cookTorrance) * radiance * max(dot(normal, lightDir), 0.0);
}

void main() {
  vec3 viewDir = normalize(-i_FragPos);

  vec3 normal = normalize(i_Normal);
  if (pc.normal_tex_index != 0) {
    normal = texture(textures[pc.normal_tex_index], i_UV).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(i_TBN * normal);
  }

  float metallic = pc.metallic;
  if (pc.metallic_tex_index != 0) {
    metallic = length(texture(textures[pc.metallic_tex_index], i_UV).rgb);
  }

  float roughness = pc.roughness;
  if (pc.roughness_tex_index != 0) {
    roughness = length(texture(textures[pc.roughness_tex_index], i_UV).rgb);
    roughness = clamp(roughness, 0.04, 1.0);
  }

  vec3 albedo = texture(textures[pc.albedo_tex_index], i_UV).rgb;

  vec3 light = vec3(0.06) * albedo;
  for (int i = 0; i < lights.lightCount; i++) {
    light += calculate_light(lights.lights[i], viewDir, normal, albedo, roughness, metallic);
  }

  light = vec3(1.0) - exp(-light * 1.0);

  o_FragColor = vec4(light, 1.0);
}
