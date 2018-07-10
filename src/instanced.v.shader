#version 330

#define MAX_PARTICLES 500

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;

out vec3 frag_pos;
out vec3 normal;
out float instance_id;

uniform mat4 uni_model_tr;
uniform mat4 uni_viewproj_tr;

uniform vec3 uni_particle_offests[MAX_PARTICLES];
uniform float uni_particle_mass[MAX_PARTICLES];

void main() {
    float size = pow(uni_particle_mass[gl_InstanceID], 1.0 / 3.0);
    vec3 adj_pos = in_vertex * size + uni_particle_offests[gl_InstanceID];
    gl_Position = uni_viewproj_tr * uni_model_tr * vec4(adj_pos, 1.0);
    frag_pos = vec3(uni_model_tr * vec4(adj_pos, 1.0));
    normal = in_normal;
    instance_id = gl_InstanceID;
}

