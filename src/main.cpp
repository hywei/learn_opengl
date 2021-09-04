#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "shader.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);

const int k_width = 800;
const int k_height = 600;

float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

uint32_t createTexture(const char* texture_file, GLenum format)
{
	uint32_t texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options ( on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	uint8_t* data = stbi_load(texture_file, &width, &height, &nrChannels, 0);
	if (data)
	{
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

Camera camera(glm::vec3(0.f, 0.f, 3.f), glm::vec3(0.f, 0.f, 0.f));

float delta_time = 0.f;
float last_frame_time = 0.f;

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

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, k_width, k_height);

	/// first, configure the cube's VAO (and VBO)
	uint32_t cubeVAO, VBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	// 1. bind Vertex Array Object
	glBindVertexArray(cubeVAO);

	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// 3. set the vertex attributes pointers
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
		(const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	/// second, configure the light's VAO (VBO stays the same; the vertices are the 
	/// same for the lgiht object which is also a 3D cube
	uint32_t lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	// we only need to bind the VBO (to link it with glVertexAttribPointer), no need
	// to fill it; the VBO's data already contains all we need (it's already bound, 
	// but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);

	Shader lightingShader(
		"E:\\projects\\learn_opengl\\shader\\shader.vs",
		"E:\\projects\\learn_opengl\\shader\\shader.fs");
	Shader lightCubeShader(
		"E:\\projects\\learn_opengl\\shader\\shader.vs",
		"E:\\projects\\learn_opengl\\shader\\light.fs");

	glm::mat4 model = glm::mat4(1.f);
	glm::mat4 view = camera.getLookAt();
	glm::mat4 projection = glm::mat4(1.f);
	projection = glm::perspective(glm::radians(45.f), (float)k_width / (float)k_height, 0.1f, 100.f);

	glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

	uint32_t frame_index = 0;
	while (!glfwWindowShouldClose(window))
	{
		float current_frame_time = static_cast<float>(glfwGetTime());
		delta_time = current_frame_time - last_frame_time;

		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 cameraPos = camera.getPosition();
		lightingShader.use();
		lightingShader.setVec3f("objectColor", 1.f, 0.5f, 0.31f);
		lightingShader.setVec3f("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.setVec3f("lightPos", lightPos.x, lightPos.y, lightPos.z);
		lightingShader.setVec3f("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

		view = camera.getLookAt();
		model = glm::mat4(1.f);

		lightingShader.setMat4fv("model", glm::value_ptr(model));
		lightingShader.setMat4fv("view", &view[0][0]);
		lightingShader.setMat4fv("projection", glm::value_ptr(projection));

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4(1.f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));

		lightCubeShader.use();
		lightCubeShader.setMat4fv("model", glm::value_ptr(model));
		lightCubeShader.setMat4fv("view", &view[0][0]);
		lightCubeShader.setMat4fv("projection", glm::value_ptr(projection));

		glBindVertexArray(lightCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();

		frame_index++;
		last_frame_time = current_frame_time;
	}

	// de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightCubeVAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
	//glDeleteProgram(shaderProgram1);
	//glDeleteProgram(shaderProgram2);

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
		camera.setPosition(camera.getPosition() - camear_speed * (glm::normalize(glm::cross(camera.getDirection(), Camera::WORLD_UP_DIR))));
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.setPosition(camera.getPosition() + camear_speed * (glm::normalize(glm::cross(camera.getDirection(), Camera::WORLD_UP_DIR))));
	}

}
