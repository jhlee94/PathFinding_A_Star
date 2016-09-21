#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform mat4 MVP;

out vec4 frag_color;

void main() {
	gl_Position = MVP * vec4(position, 1.0);
	//frag_color = normalize(position);

if(color.x == 1.0f && color.y == 1.0f && color.z == 1.0f) {
	  frag_color.rgb = vec3(1, 0, 0);
	  frag_color.a = 0.0f;
  } else if(color.x == 0.0f && color.y == 1.0f && color.z == 0.0f) {
	  frag_color.rgb = vec3(1, 1, 1);
	  frag_color.a = 1.0f;
  }
 else {
	  frag_color.rgb = color;//normalize(position);
	  frag_color.a = 1.0f;
  }
}