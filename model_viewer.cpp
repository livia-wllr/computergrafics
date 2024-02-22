// Model viewer code for the assignments in Computer Graphics 1TD388/1MD150.
//
// Modify this and other source files according to the tasks in the instructions.
//

#include "gltf_io.h"
#include "gltf_scene.h"
#include "gltf_render.h"
#include "cg_utils.h"
#include "cg_trackball.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <iostream>

// Struct for our application context
struct Context {
    int width = 512;
    int height = 512;
    GLFWwindow *window;
    gltf::GLTFAsset asset;
    gltf::DrawableList drawables;
    cg::Trackball trackball;
    GLuint program;
    GLuint emptyVAO;
    float elapsedTime;
    std::string gltfFilename = "gargo.gltf";
    // Add more variables here...
    glm::vec3 myAmbientColor = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mySpecularColor = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mySpecularPower = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 myDiffuseColor = glm::vec3(0.0f, 0.0f, 0.0f);
    bool myPerspective = false;
    bool showNormals = false;
    bool showGamma = false;
    glm::mat4 myPerspectiveMatrix = glm::mat4(1.0f);
    float myShowNormals = 0.0f;
    float myShowOrthographic = 0.0f;
    float myGammaCorrection = 0.0f;
};

// Returns the absolute path to the src/shader directory
std::string shader_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/src/shaders/";
}

// Returns the absolute path to the assets/gltf directory
std::string gltf_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/gltf/";
}

void do_initialization(Context &ctx)
{
    ctx.program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");

    gltf::load_gltf_asset(ctx.gltfFilename, gltf_dir(), ctx.asset);
    gltf::create_drawables_from_gltf_asset(ctx.drawables, ctx.asset);
}

void draw_scene(Context &ctx)
{
    // Part 4 declarations
    glm::mat4 view = glm::mat4(ctx.trackball.orient);
    glm::mat4 projection = glm::mat4(1.0f);
    //glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 model = (glm::translate(glm::mat4(1.0f), glm::vec3(1.0f))) * glm::scale(glm::mat4(1.0f), glm::vec3(100.0f));
    glm::vec3 diffuseColor = ctx.myDiffuseColor;
    glm::vec3 lightPosition = glm::vec3(0.2f, 0.3f, 0.1f);
    glm::vec3 ambientColor = ctx.myAmbientColor;
    glm::vec3 specularColor = ctx.mySpecularColor;
    glm::vec3 specularPower = ctx.mySpecularPower; 
    
    float fwidth = float(ctx.width);
    float fheight = float(ctx.height);
    float aspectRatio = fwidth / fheight;

    glm::mat4 myLookAt = glm::lookAt(glm::vec3(0.0f, 0.0f, -600.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    view = myLookAt * view;
    glm::mat4 myOrtho = glm::ortho(-fwidth / 2, fwidth / 2, fheight / 2, -fheight / 2, 10.0f, 1000.0f);
    glm::mat4 myPerspective = glm::perspective(60.0f * 3.14159f / 180.0f, aspectRatio, 10.0f, 1000.0f);
    

    //std::cout << aspectRatio;

    // Activate shader program
    glUseProgram(ctx.program);

    // Set render state
    glEnable(GL_DEPTH_TEST);  // Enable Z-buffering

    // Define per-scene uniforms
    glUniform1f(glGetUniformLocation(ctx.program, "u_time"), ctx.elapsedTime);
    
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_view"), 1, GL_FALSE, &view[0][0]);
    //glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_diffuseColor"), 1, &diffuseColor[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_lightPosition"), 1, &lightPosition[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_ambientColor"), 1, &ambientColor[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_specularColor"), 1, &specularColor[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_specularPower"), 1, &specularPower[0]);

    if (ctx.myPerspective) {
        ctx.myPerspectiveMatrix = projection * myOrtho;
    } else {
        ctx.myPerspectiveMatrix = projection * myPerspective;
    }

    if (ctx.showNormals) {
        ctx.myShowNormals = 1.0f;
    } else {
        ctx.myShowNormals = 0.0f;
    }

    if (ctx.showGamma) {
        ctx.myGammaCorrection = 1.0f;
    } else {
        ctx.myGammaCorrection = 0.0f;
    }

    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_projection"), 1, GL_FALSE, &ctx.myPerspectiveMatrix[0][0]);
    glUniform1f(glGetUniformLocation(ctx.program, "u_showNormals"), ctx.myShowNormals);
    glUniform1f(glGetUniformLocation(ctx.program, "u_enableGammaCorrection;"), ctx.myGammaCorrection);
    // Draw scene
    for (unsigned i = 0; i < ctx.asset.nodes.size(); ++i) {
        const gltf::Node &node = ctx.asset.nodes[i];
        const gltf::Drawable &drawable = ctx.drawables[node.mesh];

        // Define per-object uniforms
        // ...

        // Draw object
        glBindVertexArray(drawable.vao);
        glDrawElements(GL_TRIANGLES, drawable.indexCount, drawable.indexType,
                       (GLvoid *)(intptr_t)drawable.indexByteOffset);
        glBindVertexArray(0);
    }

    // Clean up
    cg::reset_gl_render_state();
    glUseProgram(0);
}

void do_rendering(Context &ctx)
{
    // Clear render states at the start of each frame
    cg::reset_gl_render_state();

    // Clear color and depth buffers
    glClearColor(5.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_scene(ctx);
}

void reload_shaders(Context *ctx)
{
    glDeleteProgram(ctx->program);
    ctx->program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");
}

void error_callback(int /*error*/, const char *description)
{
    std::cerr << description << std::endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_R && action == GLFW_PRESS) { reload_shaders(ctx); }
}

void char_callback(GLFWwindow *window, unsigned int codepoint)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_CharCallback(window, codepoint);
    if (ImGui::GetIO().WantTextInput) return;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx->trackball.center = glm::vec2(x, y);
        ctx->trackball.tracking = (action == GLFW_PRESS);
    }
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    if (ImGui::GetIO().WantCaptureMouse) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    cg::trackball_move(ctx->trackball, float(x), float(y));
}

void scroll_callback(GLFWwindow *window, double x, double y)
{
    // we did not get the zoom function to work 
    // Forward event to ImGui
    ImGui_ImplGlfw_ScrollCallback(window, x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

    // we did not get the zoom function to work  but this is what we tried 
    //zoomFactor -= y * 0.1f; 



    // Calculate the new FOV based on the updated zoom factor
    //float newFOV = 45.0f * zoomFactor;

    // Recalculate projection matrix with updated FOV
    //float aspectRatio = static_cast<float>(ctx.width) / ctx.height;
    //projection = glm::perspective(glm::radians(newFOV), aspectRatio, 0.1f, 100.0f);
}

void resize_callback(GLFWwindow *window, int width, int height)
{
    // Update window size and viewport rectangle
    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->width = width;
    ctx->height = height;
    glViewport(0, 0, width, height);
}

int main(int argc, char *argv[])
{
    Context ctx = Context();
    if (argc > 1) { ctx.gltfFilename = std::string(argv[1]); }

    // Create a GLFW window
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
    glfwMakeContextCurrent(ctx.window);
    glfwSetWindowUserPointer(ctx.window, &ctx);
    glfwSetKeyCallback(ctx.window, key_callback);
    glfwSetCharCallback(ctx.window, char_callback);
    glfwSetMouseButtonCallback(ctx.window, mouse_button_callback);
    glfwSetCursorPosCallback(ctx.window, cursor_pos_callback);
    glfwSetScrollCallback(ctx.window, scroll_callback);
    glfwSetFramebufferSizeCallback(ctx.window, resize_callback);

    // Load OpenGL functions
    if (gl3wInit() || !gl3wIsSupported(3, 3) /*check OpenGL version*/) {
        std::cerr << "Error: failed to initialize OpenGL" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Initialize ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(ctx.window, false /*do not install callbacks*/);
    ImGui_ImplOpenGL3_Init("#version 330" /*GLSL version*/);

    // Initialize rendering
    glGenVertexArrays(1, &ctx.emptyVAO);
    glBindVertexArray(ctx.emptyVAO);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    do_initialization(ctx);

    // Start rendering loop
    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
        ctx.elapsedTime = glfwGetTime();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

    
        ImGui::Text("GUI Window");
        ImGui::Checkbox("Orthographic Perspective", &ctx.myPerspective);
        ImGui::Checkbox("Show Surface Normals", &ctx.showNormals);
        ImGui::ColorEdit3("Gamma Correction", &ctx.myGammaCorrection);
        ImGui::ColorEdit3("Ambient Color", &ctx.myAmbientColor[0]);
        ImGui::ColorEdit3("Specular Color", &ctx.mySpecularColor[0]);
        ImGui::ColorEdit3("Specular Power", &ctx.mySpecularPower[0]);
        ImGui::ColorEdit3("Diffuse Color", &ctx.myDiffuseColor[0]);
        
        // we did not get the zoom function to work 
        glfwSetScrollCallback(ctx.window, scroll_callback);
        
        //ImGui::ShowDemoWindow();
        do_rendering(ctx);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(ctx.window);
    }

    // Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}
