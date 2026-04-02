#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_LIGHTS 128
#define MAX_TEXTURES 10000

layout(location = 0) in vec3 i_Normal;
layout(location = 1) in vec3 i_FragPos;
layout(location = 2) in vec2 i_UV;
layout(location = 3) in mat3 i_TBN;

layout(push_constant) uniform PushConstants {
  mat4 proj_mat;
  mat4 view_mat;
  mat4 model;
  int albedo_tex_index;
  int normal_tex_index;
  float diffuse;
  float specular;
} pc;

struct Light {
  vec4 position;
  vec4 direction;
  vec4 color;
};

layout(std140, set = 0, binding = 0) uniform Lights {
  Light lights[MAX_LIGHTS];
  int lightCount;
} lights;

layout(set = 1, binding = 0) uniform sampler2D textures[MAX_TEXTURES];

layout(location = 0) out vec4 o_FragColor;

vec3 directional_light(Light light, vec3 normal, vec3 viewDir, vec3 fragPos, float diffuseStrength, float specularStrength) {
  vec3 result = vec3(0);
  vec3 lightDir = normalize(-(pc.view_mat * vec4(light.direction.xyz, 0.0)).xyz);

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diffuseStrength * diff * light.color.rgb;

  vec3 halfway = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfway), 0.0), 32.0);
  vec3 specular = specularStrength * spec * light.color.rgb;

  result += diffuse + specular;
  return result;
}

vec3 point_light(Light light, vec3 normal, vec3 viewDir, vec3 fragPos, float diffuseStrength, float specularStrength) {
  vec3 lightPos = (pc.view_mat * vec4(light.position.xyz, 1.0f)).xyz;
  vec3 lightDir = normalize(lightPos - fragPos);

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diffuseStrength * diff * light.color.rgb;

  vec3 halfway = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfway), 0.0), 32.0);
  vec3 specular = specularStrength * spec * light.color.rgb;

  float distance = length(lightPos - fragPos);
  float constant = 1.0;
  float linear = 0.09;
  float quadratic = 0.032;

  float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);

  return (diffuse + specular) * attenuation;
}

void main() {
  vec3 col = texture(textures[pc.albedo_tex_index], i_UV).xyz;
  float ambientStrength = 0.2;
  vec3 ambient = vec3(1) * ambientStrength;
  vec3 lightAccumulation = vec3(0);
  float diffuse = clamp(pc.diffuse, 0.0, 1.0);
  float specular = clamp(pc.specular, 0.0, 1.0);

  vec3 normal = i_Normal;
  if (pc.normal_tex_index != 0) {
    normal = texture(textures[pc.normal_tex_index], i_UV).xyz * 2.0f - 1.0f;
    normal = normalize(i_TBN * normal);
  }

  for (int i = 0; i < lights.lightCount; i++) {
    Light light = lights.lights[i];
    if (light.position.w == 1.0f) {
      lightAccumulation += directional_light(light, normal, normalize(-i_FragPos), i_FragPos, diffuse, specular);
    } else if (light.position.w == 2.0f) {
      lightAccumulation += point_light(light, normal, normalize(-i_FragPos), i_FragPos, diffuse, specular);
    }
  }

  vec3 result = (ambient + lightAccumulation) * col;
  o_FragColor = vec4(result, 1.0);
}
