#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>

#include "input.h"

struct Clock
{
    double time {0};
    double time_increment {0};

    Clock() : time(glfwGetTime()), time_increment(0)
    {}

    double tick()
    {
        const double current_time = glfwGetTime();
        time_increment            = current_time - time;
        time                      = current_time;

        return time_increment;
    }
};

class App {

public:
    virtual int  init(const char* title, uint32_t width, uint32_t height);
    virtual void clear();
    virtual void render();
    virtual void run();

    static void size_event(GLFWwindow* window, int x, int y);
    static void key_event(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_event(GLFWwindow* window, int button_id, int action, int mods);
    static void cursor_position_event(GLFWwindow* window, double xpos, double ypos);
    static void scroll_event(GLFWwindow* window, double xoffset, double yoffset);

    static void error_callback(int error, const char* description);

private:
    GLFWwindow* window_ {nullptr};
    uint32_t    width_ {800};
    uint32_t    height_ {600};
    Input       input_;
    Clock       clock_;
    const char* title_;
};

int App::init(const char* title, uint32_t width, uint32_t height)
{
    glfwSetErrorCallback(App::error_callback);

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    width_  = width;
    height_ = height;
    title_  = title;
    input_.reset();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4); // anti-aliasing

    window_ = glfwCreateWindow(width_, height_, title, NULL, NULL);
    if (window_ == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetWindowUserPointer(window_, this);

    glfwSetKeyCallback(window_, App::key_event);
    glfwSetCursorPosCallback(window_, App::cursor_position_event);
    glfwSetMouseButtonCallback(window_, App::mouse_button_event);
    glfwSetScrollCallback(window_, App::scroll_event);
    glfwSetFramebufferSizeCallback(window_, App::size_event);

    glfwMakeContextCurrent(window_);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    return 0;
}

void App::clear()
{}

void App::run()
{
    double last_time_tick = 0;
    double increments     = 0;
    int    framerate      = 0;

    while (!glfwWindowShouldClose(window_))
    {
        clock_.tick();

        float interval = clock_.time - last_time_tick;
        increments += 1;
        if (interval > 0)
        {
            int new_framerate = roundf(increments / interval);
            if (framerate != new_framerate)
            {
                framerate = new_framerate;

                char title[256];
                snprintf(title, 256, "%s | %d fps", title_, framerate);
                glfwSetWindowTitle(window_, title);
            }
            last_time_tick = clock_.time;
            increments     = 0;
        }

        render();

        input_.consume();

        glfwPollEvents();
    }

    glfwDestroyWindow(window_);
    glfwTerminate();
}

void App::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwSwapBuffers(window_);
}

void App::size_event(GLFWwindow* window, int x, int y)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app || app->window_ != window)
        return;

    app->width_  = x;
    app->height_ = y;

    std::cout << "Info: Resolution " << x << " " << y << std::endl;

    glViewport(0, 0, x, y);
}

void App::key_event(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app || app->window_ != window)
        return;

    // hotkey mapping
}

void App::mouse_button_event(GLFWwindow* window, int button_id, int action, int mods)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app || app->window_ != window)
        return;

    // only handle the basic three mouse button
    if (button_id >= GLFW_MOUSE_BUTTON_LEFT && button_id <= GLFW_MOUSE_BUTTON_MIDDLE)
    {
        double now = glfwGetTime();
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (action == GLFW_PRESS)
        {
            app->input_.setButtonEvent(
                button_id, Input::Action::press, mods, glm::vec2(xpos, ypos), now);
        }
        else if (action == GLFW_RELEASE)
        {
            app->input_.setButtonEvent(
                button_id, Input::Action::release, mods, glm::vec2(xpos, ypos), now);

            // to exit any special pointer mode, restore the GLFW_CURSOR_NORMAL cursor mode
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void App::cursor_position_event(GLFWwindow* window, double xpos, double ypos)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app || app->window_ != window)
        return;

    app->input_.setPointerPosition(glm::vec2(xpos, ypos));
}

void App::scroll_event(GLFWwindow* window, double xoffset, double yoffset)
{
    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app || app->window_ != window)
        return;
}

void App::error_callback(int error, const char* description)
{
    std::cerr << "Error:: " << description << std::endl;
}
