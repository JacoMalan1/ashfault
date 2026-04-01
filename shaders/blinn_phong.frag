#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_LIGHTS 128
#define MAX_TEXTURES 10000

layout(location = 0) in vec3 i_Normal;
layout(location = 1) in vec3 i_FragPos;
layout(location = 2) in vec2 i_UV;
layout(location = 3) flat in int i_TexIndex;
layout(location = 4) flat in int i_NormalIdx;
layout(location = 5) flat in float i_Diffuse;
layout(location = 6) flat in float i_Specular;
layout(location = 9) in mat4 i_ViewMat;
layout(location = 13) in mat3 i_TBN;

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
  vec3 lightDir = normalize(-(i_ViewMat * vec4(light.direction.xyz, 0.0)).xyz);

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diffuseStrength * diff * light.color.rgb;

  vec3 halfway = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfway), 0.0), 32.0);
  vec3 specular = specularStrength * spec * light.color.rgb;

  result += diffuse + specular;
  return result;
}

vec3 point_light(Light light, vec3 normal, vec3 viewDir, vec3 fragPos, float diffuseStrength, float specularStrength) {
  vec3 lightPos = (i_ViewMat * vec4(light.position.xyz, 1.0f)).xyz;
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
  vec3 col = texture(textures[i_TexIndex], i_UV).xyz;
  float ambientStrength = 0.2;
  vec3 ambient = vec3(1) * ambientStrength;
  vec3 lightAccumulation = vec3(0);
  float diffuse = clamp(i_Diffuse, 0.0, 1.0);
  float specular = clamp(i_Specular, 0.0, 1.0);

  vec3 normal = i_Normal;
  if (i_NormalIdx != 0) {
    normal = texture(textures[i_NormalIdx], i_UV).xyz * 2.0f - 1.0f;
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
