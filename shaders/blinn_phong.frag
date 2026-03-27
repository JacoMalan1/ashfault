#version 450

#define MAX_LIGHTS 128

layout(location = 0) in vec3 i_Normal;
layout(location = 1) in vec3 i_FragPos;
layout(location = 2) in mat4 i_ViewMat;

struct Light {
  vec4 position;
  vec4 direction;
  vec4 color;
};

layout(std140, set = 0, binding = 0) uniform Lights {
  Light lights[MAX_LIGHTS];
  int lightCount;
} lights;

layout(location = 0) out vec4 o_FragColor;

vec3 directional_light(Light light, vec3 normal, vec3 viewDir, vec3 fragPos) {
  vec3 result = vec3(0);
  vec3 lightDir = normalize(-light.direction.xyz);

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diff * light.color.rgb;

  vec3 halfway = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfway), 0.0), 32.0);
  vec3 specular = spec * light.color.rgb;

  result += diffuse + specular;
  return result;
}

vec3 point_light(Light light, vec3 normal, vec3 viewDir, vec3 fragPos) {
  vec3 viewNormal = normalize(mat3(transpose(inverse(i_ViewMat))) * normal);
  vec3 lightPos = (i_ViewMat * vec4(light.position.xyz, 1.0f)).xyz;
  vec3 lightDir = normalize(lightPos - fragPos);

  float diff = max(dot(viewNormal, lightDir), 0.0);
  vec3 diffuse = diff * light.color.rgb;

  vec3 halfway = normalize(lightDir + viewDir);
  float spec = pow(max(dot(viewNormal, halfway), 0.0), 32.0);
  vec3 specular = spec * light.color.rgb;

  float distance = length(lightPos - fragPos);
  float constant = 1.0;
  float linear = 0.09;
  float quadratic = 0.032;

  float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);

  return (diffuse + specular) * attenuation;
}

void main() {
  vec3 col = vec3(1);
  float ambientStrength = 0.2;
  vec3 ambient = vec3(1) * ambientStrength;
  vec3 lightAccumulation = vec3(0);

  for (int i = 0; i < lights.lightCount; i++) {
    Light light = lights.lights[i];
    if (light.position.w == 1.0f) {
      lightAccumulation += directional_light(light, i_Normal, normalize(-i_FragPos), i_FragPos);
    } else if (light.position.w == 2.0f) {
      lightAccumulation += point_light(light, i_Normal, normalize(-i_FragPos), i_FragPos);
    }
  }

  vec3 result = (ambient + lightAccumulation) * col;
  o_FragColor = vec4(result, 1.0);
}
