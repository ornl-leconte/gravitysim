#version 330 core

layout(location = 0) in vec3 v_vertex;
layout(location = 1) in vec3 v_normal;
// packed x, y, z, mass
layout(location = 2) in vec4 v_particle;
layout(location = 3) in vec4 v_col;

out vec3 f_normal;
out vec3 f_fragpos;

out vec3 f_lightpos;
out vec4 f_col;
flat out int f_instanceid;

uniform mat4 u_model_tr;
uniform mat4 u_viewproj_tr;


void main() {
    float size = pow(v_particle.w, 1.0 / 3.0);
   // float size = 1.0;
    vec4 adj_pos = vec4(v_vertex * size + v_particle.xyz, 1.0);

    gl_Position = u_viewproj_tr * u_model_tr * adj_pos;

    f_normal = v_normal;
    f_fragpos = vec3(u_model_tr * adj_pos);
    f_col = v_col;
    
    f_instanceid = gl_InstanceID;
}

