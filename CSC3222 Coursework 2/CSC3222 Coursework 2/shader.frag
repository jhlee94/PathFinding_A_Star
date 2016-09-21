#version 330 core
in vec4 frag_color;

out vec4 color;

void main(){
  //color.xyz = vec3(0.1f * frag_path[1], 0.1f*frag_path[2], 0.1f*frag_path[3]);
  //color.w = 1.0f;
  color = frag_color;
}