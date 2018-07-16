#version 330 core

uniform vec3 u_lightpos;

in vec3 f_normal;
in vec3 f_fragpos;

in vec4 f_col;

flat in int f_instanceid;

out vec3 f_color;

void main(){
  vec3 norm = normalize(f_normal);
  vec3 light_dir = normalize(u_lightpos - f_fragpos);

  float diff = max(dot(norm, light_dir), 0.0);
  f_color = diff * f_col.xyz;
}
