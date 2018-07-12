#version 330 core

in vec3 normal;
in vec3 frag_pos;
out vec3 color;

void main(){
  color = vec3(0.0, 1.0, 1.0);
  //color = vec3(1.0, 1.0, 1.0);
}
