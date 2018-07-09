#version 330 core

uniform vec3 uni_lightpos;

in vec3 normal;
in vec3 frag_pos;
out vec3 color;


void main(){
  vec3 norm = normalize(normal);
  vec3 light_dir = normalize(uni_lightpos - frag_pos);
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = diff * vec3(1.0, 1.0, 0.0);
  color = diffuse;
  //color = vec3(1.0, 1.0, 1.0);
}
