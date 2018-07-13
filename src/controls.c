
#include "gravitysim.h"

#include "ccgl_gl.h"

#include "render.h"

// initialization
bool * is_pressed_state;


void control_init() {
    is_pressed_state = malloc(sizeof(bool) *  (GLFW_KEY_LAST + 1));

    int i;
    for (i = 0; i < GLFW_KEY_LAST + 1; ++i) {
        is_pressed_state[i] = GLFW_RELEASE;
    }

}

bool pressed_once(int k) {
    bool res = (glfwGetKey(window, k) == GLFW_PRESS) && (is_pressed_state[k] != GLFW_PRESS);
    is_pressed_state[k] = glfwGetKey(window, k);
    return res;
}

bool is_pressed(int k) {
    is_pressed_state[k] = glfwGetKey(window, k);
    return glfwGetKey(window, k) == GLFW_PRESS;
}


void control_update() {
    // will be 1.0 at perfect 60.0 fps
    float cm = GS.dt * 60.0;
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

    // do update for all sorts of controllers 
    if (is_pressed(GLFW_KEY_D)) scene.cam_period += (cm / 60.0) * (0.666 * M_PI);
    if (is_pressed(GLFW_KEY_A)) scene.cam_period -= (cm / 60.0) * (0.666 * M_PI);
    if (is_pressed(GLFW_KEY_W)) scene.cam_pitch += (cm / 60.0) * (0.666 * M_PI);
    if (is_pressed(GLFW_KEY_S)) scene.cam_pitch -= (cm / 60.0) * (0.666 * M_PI);
    if (is_pressed(GLFW_KEY_SPACE)) scene.cam_dist /= expf(0.5 * (cm / 60.0));
    if (is_pressed(GLFW_KEY_LEFT_SHIFT) || is_pressed(GLFW_KEY_RIGHT_SHIFT)) scene.cam_dist *= expf(0.5 * (cm / 60.0));

    if (is_pressed(GLFW_KEY_PERIOD)) scene.cam_fov += M_PI * 0.15 * (cm / 60.0);
    if (is_pressed(GLFW_KEY_COMMA)) scene.cam_fov -= M_PI * 0.15 * (cm / 60.0);

    if (pressed_once(GLFW_KEY_P)) GS.is_paused = !GS.is_paused;


    if (scene.cam_pitch > M_PI / 2) scene.cam_pitch = M_PI / 2 - 0.01;
    if (scene.cam_pitch < - M_PI / 2) scene.cam_pitch = - M_PI / 2 + 0.01;

    if (scene.cam_fov > M_PI) scene.cam_fov = M_PI - 0.01;
    if (scene.cam_fov < 0.0) scene.cam_fov = 0.01;


}
