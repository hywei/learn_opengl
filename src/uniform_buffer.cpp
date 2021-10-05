#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>

#include "camera.h"
#include "light.h"
#include "model.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);

uint32_t createTexture(const char* texture_file);

uint32_t loadCubemap(std::vector<std::string> faces);

const int k_width  = 1600;
const int k_height = 900;

float delta_time      = 0.f;
float last_frame_time = 0.f;

Camera camera(glm::vec3(0.f, 0.f, 3.f), glm::vec3(0.f, 0.f, 0.f));
Light  light;

float cube_vertices[] = {
    // positions
    -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
    0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,

    -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,

    0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,
    0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,
    0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,

    -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f,
};

int main()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(k_width, k_height, "LearnOpenGL", NULL, NULL);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, k_width, k_height);

    Shader shader_red("../../../shader/uniform_buffer.vs", "../../../shader/uniform_buffer_red.fs");
    Shader shader_blue("../../../shader/uniform_buffer.vs",
                       "../../../shader/uniform_buffer_blue.fs");
    Shader shader_green("../../../shader/uniform_buffer.vs",
                        "../../../shader/uniform_buffer_green.fs");
    Shader shader_yellow("../../../shader/uniform_buffer.vs",
                         "../../../shader/uniform_buffer_yellow.fs");

    uint32_t cube_vao, cube_vbo;
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &cube_vbo);
    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), &cube_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // configure a uniform buffer object
    // 1. get the relevant block indices
    uint32_t uniform_block_index_red    = glGetUniformBlockIndex(shader_red.ID, "Matrices");
    uint32_t uniform_block_index_blue   = glGetUniformBlockIndex(shader_blue.ID, "Matrices");
    uint32_t uniform_block_index_green  = glGetUniformBlockIndex(shader_green.ID, "Matrices");
    uint32_t uniform_block_index_yellow = glGetUniformBlockIndex(shader_yellow.ID, "Matrices");
    // 2. link each shader's uniform block to this uniform binding point
    glUniformBlockBinding(shader_red.ID, uniform_block_index_red, 0);
    glUniformBlockBinding(shader_blue.ID, uniform_block_index_blue, 0);
    glUniformBlockBinding(shader_green.ID, uniform_block_index_green, 0);
    glUniformBlockBinding(shader_yellow.ID, uniform_block_index_yellow, 0);
    // 3. create the uniform buffer
    uint32_t ubo_matrices;
    glGenBuffers(1, &ubo_matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // define the rage of the buffer that links to a uniform binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_matrices, 0, 2 * sizeof(glm::mat4));

    glm::mat4 projection =
        glm::perspective(glm::radians(45.f), (float)k_width / (float)k_height, 0.1f, 100.f);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glm::mat4 view  = glm::mat4(1.f);
    glm::mat4 model = glm::mat4(1.f);

    uint32_t frame_index = 0;
    while (!glfwWindowShouldClose(window))
    {
        float current_frame_time = static_cast<float>(glfwGetTime());
        delta_time               = current_frame_time - last_frame_time;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = camera.getLookAt();
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices);
        glBufferSubData(
            GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // draw cubes;
        glBindVertexArray(cube_vao);
        shader_red.use();
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f));
        shader_red.setMat4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        shader_green.use();
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f));
        shader_green.setMat4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        shader_blue.use();
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(0.75f, -0.75f, 0.0f));
        shader_blue.setMat4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        shader_yellow.use();
        model = glm::mat4(1.f);
        model = glm::translate(model, glm::vec3(-0.75f, -0.75f, 0.0f));
        shader_yellow.setMat4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_index++;
        last_frame_time = current_frame_time;
    }

    glDeleteVertexArrays(1, &cube_vao);
    glDeleteBuffers(1, &cube_vbo);

    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float camear_speed = 2.5f * delta_time;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.setPosition(camera.getPosition() + camear_speed * camera.getDirection());
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.setPosition(camera.getPosition() - camear_speed * camera.getDirection());
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.setPosition(camera.getPosition() -
                           camear_speed * (glm::normalize(glm::cross(camera.getDirection(),
                                                                     Camera::WORLD_UP_DIR))));
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.setPosition(camera.getPosition() +
                           camear_speed * (glm::normalize(glm::cross(camera.getDirection(),
                                                                     Camera::WORLD_UP_DIR))));
    }
}

uint32_t createTexture(const char* texture_file)
{
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options ( on the currently bound
    // texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int      width, height, nrChannels;
    uint8_t* data = stbi_load(texture_file, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

uint32_t loadCubemap(std::vector<std::string> faces)
{
    uint32_t texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    int width, height, nr_channels;
    for (size_t index = 0; index < faces.size(); index++)
    {
        unsigned char* data = stbi_load(faces[index].c_str(), &width, &height, &nr_channels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index,
                         0,
                         GL_RGB,
                         width,
                         height,
                         0,
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load cubemap texture at path: " << faces[index] << std::endl;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture_id;
}