
#version 430
in vec4 pos;
in vec4 norm;
out vec4 Pos;
out vec4 Norm;
uniform mat4 view;
uniform mat4 rot;
void main() {
  Pos = pos;
  Norm = normalize(pos);
  //gl_Position = view*rot*vec4(pos.xy,0,1);
  gl_Position = view*rot*pos;
}
