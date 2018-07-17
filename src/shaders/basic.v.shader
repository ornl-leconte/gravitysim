#version 330

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;

out vec3 frag_pos;
out vec3 normal;

uniform mat4 uni_model_tr;
uniform mat4 uni_viewproj_tr;


void main() {
    gl_Position = uni_viewproj_tr * uni_model_tr * vec4(in_vertex, 1.0);
    frag_pos = vec3(uni_model_tr * vec4(in_vertex, 1.0));
    normal = in_normal;
    //f_col = v_col;
}

