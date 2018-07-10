#version 330

#define MAX_PARTICLES 4096

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;

out vec3 frag_pos;
out vec3 normal;
out float instance_id;

uniform mat4 uni_model_tr;
uniform mat4 uni_viewproj_tr;

uniform vec3 uni_particle_offests[MAX_PARTICLES];

void main() {
    vec3 adj_pos = in_vertex + uni_particle_offests[gl_InstanceID];
    gl_Position = uni_viewproj_tr * uni_model_tr * vec4(adj_pos, 1.0);
    frag_pos = vec3(uni_model_tr * vec4(adj_pos, 1.0));
    normal = in_normal;
    instance_id = gl_InstanceID;
}

