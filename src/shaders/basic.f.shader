#version 330 core

uniform vec3 uni_light;
uniform vec3 uni_col;


in vec3 normal;
in vec3 frag_pos;
out vec3 color;



void main(){
  vec3 norm = normalize(normal);
  vec3 light_dir = normalize(uni_light - frag_pos);
  float diff = max(dot(norm, light_dir), 0.0);
  
  color = diff * uni_col;
  //color = vec3(1.0, 0.0, 0.0);
}
