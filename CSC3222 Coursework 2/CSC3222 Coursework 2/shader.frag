#version 330 core
in vec3 frag_color;
out vec3 color;

void main(){
  color = vec3(1.f, 1.f, 1.f);
  //color = frag_color;
}