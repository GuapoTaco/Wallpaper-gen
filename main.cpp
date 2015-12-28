#include "tiny_obj_loader.h"
#include "lodepng.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <array>
#include <random>

std::array<float, 3> fromRGB(unsigned char r, unsigned char g, unsigned char b)
{
	return{ (float)r / 255.f, (float)g / 255.f, (float)b / 255.f };
}


std::array<float, 3> fromHex(uint32_t color)
{
	return fromRGB(color >> 24, color << 8 >> 24, color << 16 >> 24);
}

GLuint program, vertLocs, vertArray, indicies, colors, numElements;

void setup()
{



	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> mats;

	std::string error;

	// load model
	tinyobj::LoadObj(shapes, mats, error, "G:/home/russellg/projects/Wallpaper/thing.obj");

	if (!shapes.size())
	{
		std::cout << error << std::endl;

		exit(-1);
	}



	tinyobj::mesh_t& mesh = shapes[0].mesh;

	numElements = mesh.indices.size();

	//setup buffer
	glGenVertexArrays(1, &vertArray);
	glBindVertexArray(vertArray);


	glGenBuffers(1, &vertLocs);
	glBindBuffer(GL_ARRAY_BUFFER, vertLocs);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.positions.size(), mesh.positions.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &indicies);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicies);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);


	std::array<std::array<float, 3>, 8> colorsToChooseFrom =
	{
		fromHex(0xda253900),
		fromHex(0x377ac800),
		fromHex(0x5d3da300),
		fromHex(0xe3903100),
		fromHex(0x7dbe3200),
		fromHex(0x38579900),
		fromHex(0x89619e00),
		fromHex(0x19191900)
	};

	// generate colors
	std::vector<std::array<float, 3>>  colorsData(mesh.positions.size() / 3);

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<> distr(0, colorsToChooseFrom.size() - 1);

	for (std::array<float, 3>& color : colorsData)
	{

		color = colorsToChooseFrom[distr(gen)];
	}

	glGenBuffers(1, &colors);
	glBindBuffer(GL_ARRAY_BUFFER, colors);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * colorsData.size(), colorsData.data(), GL_STATIC_DRAW);

	// load shader
	auto vertShader =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vertLocationIn;\n"
		"layout(location = 1) in vec3 vertColors;\n"
		"uniform mat4 MVP;\n"
		"\n"
		"out vec3 color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = MVP * vec4(vertLocationIn, 1.f);\n"
		"	\n"
		"	color = vertColors;\n"
		"}\n";

	auto fragShader =
		"#version 330 core\n"
		"in vec3 color;\n"
		"out vec3 fragColor;\n"
		"void main()\n"
		"{\n"
		"	fragColor = color;\n"
		"}\n";

	// Create the shaders
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	int32_t Result = GL_FALSE;
	int InfoLogLength;

	glShaderSource(vertexShader, 1, &vertShader, nullptr);
	glCompileShader(vertexShader);

	// Check Vertex Shader
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 1) {
		auto VertexShaderErrorMessage = std::vector<char>(InfoLogLength + 1);
		glGetShaderInfoLog(vertexShader, InfoLogLength, nullptr, VertexShaderErrorMessage.data());
		std::cout << VertexShaderErrorMessage.data();
	}
	else
	{
		std::cout << "\tShader Successfully Compiled";
	}

	// Compile Fragment Shader
	glShaderSource(fragmentShader, 1, &fragShader, nullptr);
	glCompileShader(fragmentShader);

	// Check Fragment Shader
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 1) {
		auto FragmentShaderErrorMessage = std::vector<char>(InfoLogLength + 1);
		glGetShaderInfoLog(fragmentShader, InfoLogLength, nullptr, FragmentShaderErrorMessage.data());
		std::cout << FragmentShaderErrorMessage.data();
	}
	else
	{
		std::cout << "\tShader Successfully Compiled";
	}

	// Link the program
	std::cout << "\tLinking program ";
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// Check the program
	glGetProgramiv(program, GL_LINK_STATUS, &Result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 1) {
		auto ProgramErrorMessage = std::vector<char>(InfoLogLength + 1);
		glGetProgramInfoLog(program, InfoLogLength, nullptr, ProgramErrorMessage.data());
		std::cout << ProgramErrorMessage.data();
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	std::cout << "\tSuccessfully Linked Program.";

}

void renderAndStuff()
{
	glBindVertexArray(vertArray);

	// render!
	glm::mat4 viewMat = glm::lookAt(glm::vec3{ 0.f, 80.f, 40.f }, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 1.f });
	glm::mat4 projectionMat = glm::perspective(20.f, 16.f / 9.f, .1f, 300.f);

	glm::mat4 MVPMat = projectionMat * viewMat;

	glm::vec4 a = MVPMat * glm::vec4(-62.8301315, 0.548247993, 51.5535278, 1);

	glUniformMatrix4fv(glGetUniformLocation(program, "MVP"), 1, GL_FALSE, &MVPMat[0][0]);


	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, vertLocs);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);


	glBindBuffer(GL_ARRAY_BUFFER, colors);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicies);
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, nullptr);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

}


int main()
{
	assert(glfwInit() == GL_TRUE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	auto window = glfwCreateWindow(1920, 1080, "Wallpaper", nullptr, nullptr);
	assert(window);

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	assert(glewInit() == GLEW_OK);


	setup();

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		renderAndStuff();

		glfwPollEvents();
		glfwSwapBuffers(window);
	}


}