
#include "gravitysim.h"

#include "render.h"
#include "math.h"


#include <stdlib.h>

int win_width = 640, win_height = 480;

GLFWwindow *window = NULL;



void error_callback(int error, const char* description) {
    log_error("Error[%d]: %s\n", error, description);
    exit(error);
}

void render_init() {
    // initializing glfw
    if (glfwInit() != 1) {                                                  
        printf("Glfw failed to init\n");     
        exit(1);                           
    }

    glfwSetErrorCallback(error_callback);

    // it's important to create a window BEFORE intiailizing glew or anything
    window = glfwCreateWindow(win_width, win_height, "gravitysim", NULL, NULL);
    glfwMakeContextCurrent(window);
    
    if (window == NULL) {
        printf("GLFW failed to create a window\n");
        exit(1);
    }

    // for some reason we need this to be set
    glewExperimental = 1;                                               
    if (glewInit() != GLEW_OK) {                                        
        printf("GLEW failed to init\n");   
        exit(1);                      
    }

    log_info("OpenGL version: %s", glGetString(GL_VERSION));
    log_debug("OpenGL Extensions: %s", glGetString(GL_EXTENSIONS));


    /* basic opengl settings */
    
    glEnable ( GL_DEPTH_TEST );


    /* initialized cached stuff, like instances */

}

bool render_update() {
    if (glfwWindowShouldClose(window)) {
        printf("Window closing...\n");
        return false;
    }

    glClearDepth( 1.0 );
    glClearColor(1.0, 0.0, 0.5, 1.0); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    /* now just normal glfw stuff */

    glfwSwapBuffers(window);
    glfwPollEvents();

    return true;
}


