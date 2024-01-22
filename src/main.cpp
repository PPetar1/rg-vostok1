#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#include "cmath"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    bool FollowModeEnabled = false;
    bool enable_fong = false;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); //We use face culling, in this way the side of the models not facing us is not rendered
    glEnable(GL_BLEND); //Enabling blending to render clouds properly
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/vertex_shader.vs", "resources/shaders/fragment_shader.fs");
    Shader sunShader("resources/shaders/vertex_shader.vs", "resources/shaders/sun_fragment_shader.fs");

    // load models
    // -----------
    Model earth_model("resources/objects/earth/scene.gltf");
    earth_model.SetShaderTextureNamePrefix("material.");

    Model clouds_model("resources/objects/clouds/scene.gltf");
    clouds_model.SetShaderTextureNamePrefix("material.");

    Model vostok_model("resources/objects/vostok/scene.gltf");
    vostok_model.SetShaderTextureNamePrefix("material.");

    Model moon_model("resources/objects/moon/scene.gltf");
    moon_model.SetShaderTextureNamePrefix("material.");

    Model sun_model("resources/objects/sun/scene.gltf");
    sun_model.SetShaderTextureNamePrefix("");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.05, 0.05, 0.05);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(0.5, 0.5, 0.5);

    pointLight.constant = 1.0f;
    pointLight.linear = 1.0/30000;
    pointLight.quadratic = 1.0/(30000*30000);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //if follow mode is enabled set camera position to follow the capsule
        glm::mat4 model;
        if (programState->FollowModeEnabled){
            model = glm::mat4(1.0f);
            model = glm::rotate(model, (float)(currentFrame/(800*0.06)), glm::vec3(-1.0,2.0,0.0)); //Adding rotation around the earth
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.045f));

            programState->camera.Position = glm::vec3(model * glm::vec4(0.0, 0.0, 0.0, 1.0));
        }

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        pointLight.position = glm::vec3(0.0, 0.0, 2345.0);
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 8.0f);
        ourShader.setInt("enable_fong", programState->enable_fong);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.002f, 3000.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


        // earth model radius 1, moon model radius 1, vostok model radius ~ 1.3, sun model radius 1
        // earth radius - 6378 km = 1, 23*(M_PI/180) tilt of orbit, vostok orbit ~ 250km (6628 km) = 1.04, vostok size 0.005 km = 0.0000008, vostok orbital period = 0,06 d
        // moon orbit 384 000 km = 60, moon radius 0.27 of earth, moon orbital period = 29 d, 24*(M_PI/180) tilt of orbit (to equator of earth)
        // sun distance 149,600,000 km = 23455, sun radius 109 x earth radius
        //sun rendering
        //sun size and distance not correct - due to float precision there were some glitches when put to proper values; Sun is here 10x closer and scaled to look ok
        sunShader.use();
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2345.0f));
        model = glm::scale(model, glm::vec3(20.0));
        sunShader.setMat4("model", model);

        sun_model.Draw(sunShader);

        // earth and clouds rendering
        ourShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, (float)(currentFrame/800), glm::vec3(0.0,1.0,0.0)); //Implementing Earth rotation around its axis
        model = glm::rotate(model, (float)(-M_PI/2), glm::vec3(1.0,0.0,0.0)); //Fixing model wrong orientation
        model = glm::scale(model, glm::vec3(1));
        ourShader.setMat4("model", model);

        earth_model.Draw(ourShader);

        float distance_to_camera = glm::distance(programState->camera.Position, glm::vec3(model * glm::vec4(0.0, 0.0, 0.0, 1.0)));//fix to z fighting
        if (distance_to_camera < 75) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::rotate(model, (float) (currentFrame / 800),
                                glm::vec3(0.0, 1.0, 0.0)); //Implementing Earth rotation around its axis
            model = glm::rotate(model, (float) (-M_PI / 2), glm::vec3(1.0, 0.0, 0.0)); //Fixing model wrong orientation
            model = glm::scale(model, glm::vec3(1.002 + (distance_to_camera / 400)));
            ourShader.setMat4("model", model);

            clouds_model.Draw(ourShader);
        }


        //vostok rendering
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

        model = glm::rotate(model, (float)(currentFrame/(800*0.06)), glm::vec3(-1.0,2.0,0.0)); //Adding rotation around the earth
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.04f));//orbit made slightly bigger because it looks nicer
        model = glm::rotate(model, (float)((currentFrame+(800*0.1*3))/(800*0.1)), glm::vec3(-1.0,2.0,-3.0)); //Adding small rotation to the model
        model = glm::scale(model, glm::vec3(1*0.00008));//Model is bigger than it should be to avoid float precision issues
        ourShader.setMat4("model", model);

        vostok_model.Draw(ourShader);

        //moon rendering
        model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)((currentFrame+(800*29))/(800*29)), glm::vec3(sin((float)(24*(M_PI/180))),cos((float)(24*(M_PI/180))),0.0)); //adding rotation around the earth

        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 60.0));

        model = glm::rotate(model, (float)(currentFrame/(800*29)), glm::vec3(0.0,1.0,0.0)); //adding rotation around itself
        model = glm::rotate(model, (float)(-M_PI/2), glm::vec3(1.0,0.0,0.0)); //Fixing model wrong orientation
        model = glm::scale(model, glm::vec3(0.27));
        ourShader.setMat4("model", model);

        moon_model.Draw(ourShader);

        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (!programState->FollowModeEnabled) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(FORWARD, deltaTime);

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(BACKWARD, deltaTime);

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(LEFT, deltaTime);

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(RIGHT, deltaTime);

    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        programState->camera.ProcessRotation(-1.0);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        programState->camera.ProcessRotation(1.0);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::DragFloat("Movement speed", &programState->camera.MovementSpeed);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::Checkbox("Follow capsule", &programState->FollowModeEnabled);
        ImGui::Checkbox("Enable fong", &programState->enable_fong);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}
