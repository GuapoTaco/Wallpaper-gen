#include "GLWidget.h"

#include "tiny_obj_loader.h"
#include "lodepng.h"

#include "Window.h"

#include <iostream>
#include <vector>

#include <glm/gtx/transform.hpp>
#include <QApplication>


GLWidget::GLWidget ( Window* parent, Qt::WindowFlags f ) : QOpenGLWidget ( nullptr, f ), owningWindow(parent)
{
	
	setFocusPolicy(Qt::ClickFocus);
	
	connect(&time, &QTimer::timeout, [this]
		{
			this->update();
		});
	time.start(0);
	
	QSurfaceFormat format;
	format.setMajorVersion(3);
	format.setMinorVersion(3);
	
	format.setSamples(8);
	
	setFormat(format);
	
}

bool GLWidget::event (QEvent* event)
{

	switch(event->type())
	{
	case QEvent::Type::MouseButtonPress:
		{
			auto mousePressEvent = static_cast<QMouseEvent*>(event);
			if(mousePressEvent->button() == Qt::LeftButton)
			{
				isMouseDown = true;
				lastMousePos = { mousePressEvent->x(), mousePressEvent->y() };
				return true;
			}
		}
		break;
	case QEvent::Type::MouseButtonRelease:
		{
			auto mousePressEvent = static_cast<QMouseEvent*>(event);
			if(mousePressEvent->button() == Qt::LeftButton)
			{
				isMouseDown = false;
				return true;
			}
		}
		break;
	case QEvent::Type::MouseMove:
		{
			auto mousePressEvent = static_cast<QMouseEvent*>(event);
			if(isMouseDown)
			{
				glm::ivec2 currentMouse = { mousePressEvent->x(), mousePressEvent->y() };
				
				glm::ivec2 deltaMouse = currentMouse - lastMousePos;
				
				auto forVecIntX = glm::rotate((float)deltaMouse.x / 1000.f, glm::vec3{0.f, 0.f, 1.f}) * glm::vec4(forwardVector, 0.f);
				forwardVector = {forVecIntX.x, forVecIntX.y, forVecIntX.z};
				
				auto forVecIntY = glm::rotate(-(float)deltaMouse.y / 1000.f, glm::vec3{1.f, 0.f, 0.f}) * glm::vec4(forwardVector, 0.f);
				forwardVector = {forVecIntY.x, forVecIntY.y, forVecIntY.z};
				
				lastMousePos = currentMouse;
				
				return true;
			}
		}
		break;
	default: break;
	}

	return QOpenGLWidget::event(event);
}


std::array<float, 3> fromRGB(unsigned char r, unsigned char g, unsigned char b)
{
	return{ (float)r / 255.f, (float)g / 255.f, (float)b / 255.f };
}

void GLWidget::markForRegeneration()
{
	needsRegenerate = true;
}

void GLWidget::markForSave(const QString& saveDest)
{
	needsSave = true;
	savePath = saveDest;
}

void GLWidget::markForColorRefresh()
{
	needsColorRefresh = true;
}




std::array<float, 3> fromHex(uint32_t color)
{
 return fromRGB(color >> 24, color << 8 >> 24, color << 16 >> 24);
}




void GLWidget::keyPressEvent ( QKeyEvent* event )
{
	QWidget::keyPressEvent ( event );
		
	switch(event->key())
	{
	case Qt::Key_Q:
		velocity.z += speed; break;
	case Qt::Key_E:
		velocity.z -= speed; break;
	case Qt::Key_A:
		velocity.x += speed; break;
	case Qt::Key_D:
		velocity.x -= speed; break;
	case Qt::Key_W:
		velocity.y += speed; break;
	case Qt::Key_S:
		velocity.y -= speed; break;
	case Qt::Key_F:
		owningWindow->setFixedSize(4096, 2304); break;
	}
	
	
}

void GLWidget::keyReleaseEvent ( QKeyEvent* event )
{
	switch(event->key())
	{
	case Qt::Key_Q:
		velocity.z -= speed; break;
	case Qt::Key_E:
		velocity.z += speed; break;
	case Qt::Key_A:
		velocity.x -= speed; break;
	case Qt::Key_D:
		velocity.x += speed; break;
	case Qt::Key_W:
		velocity.y -= speed; break;
	case Qt::Key_S:
		velocity.y += speed; break;
	}
}


void GLWidget::initializeGL()
{
	QOpenGLWidget::initializeGL();
	
	initializeOpenGLFunctions();
	
	
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> mats;

	std::string error;

	// load model
	tinyobj::LoadObj(shapes, mats, error, "thing.obj");

	if (!shapes.size())
	{
		std::cout << error << std::endl;

        std::cin.get();
	}



	tinyobj::mesh_t& mesh = shapes[0].mesh;

	numElements = mesh.indices.size();
	numVerts = mesh.positions.size() / 3;

	//setup buffer
	glGenVertexArrays(1, &vertArray);
	glBindVertexArray(vertArray);


	glGenBuffers(1, &vertLocs);
	glBindBuffer(GL_ARRAY_BUFFER, vertLocs);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.positions.size(), mesh.positions.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &indicies);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicies);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);



	glGenBuffers(1, &colors);
	glBindBuffer(GL_ARRAY_BUFFER, colors);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh.positions.size(), 0, GL_STATIC_DRAW);
	regenerate();

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
		"uniform int isRender = 0;\n"
		"out vec3 fragColor;\n"
		"uniform vec3 lineColor;"
		"void main()\n"
		"{\n"
		"	fragColor = isRender != 0 ? lineColor : color;\n"
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

	
	glEnable(GL_DEPTH_TEST);
}

void GLWidget::paintGL()
{
	QOpenGLWidget::paintGL();

	std::chrono::time_point<std::chrono::system_clock> currentTickTime = std::chrono::system_clock::now();
	
	std::chrono::duration<double> deltaDuration = currentTickTime - lastTickTime;
	float deltaTime = deltaDuration.count();
	lastTickTime = currentTickTime;
	
	
	deltaTime = fabs(deltaTime);
	
	// apply velocity
	location += velocity.y * deltaTime * forwardVector;
	location += velocity.x * deltaTime * glm::cross(forwardVector, upVector);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if(needsRegenerate) regenerate();
	needsRegenerate = false;
	
	if(needsColorRefresh)
	{
		lastSeed--;
		regenerate();
		needsColorRefresh = false;
	}
	
	glBindVertexArray(vertArray);

	
	// render!
	glm::mat4 viewMat = glm::lookAt(location, location + forwardVector, upVector);
	glm::mat4 projectionMat = glm::perspective(50.f, (float)width() / height(), .1f, 100.f);

	glm::mat4 MVPMat = projectionMat * viewMat;

	glm::vec4 a = MVPMat * glm::vec4(-62.8301315, 0.548247993, 51.5535278, 1);


    glUseProgram(program);

    auto MVPUni = glGetUniformLocation(program, "MVP");
    glUniformMatrix4fv(MVPUni, 1, GL_FALSE, &MVPMat[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, vertLocs);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);


	glBindBuffer(GL_ARRAY_BUFFER, colors);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUniform1i(glGetUniformLocation(program, "isRender"), GL_FALSE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicies);
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, nullptr);

	glLineWidth(lineSize);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniform1i(glGetUniformLocation(program, "isRender"), GL_TRUE);
	glm::vec3 lineColorVec3 = {(float)lineColor.red() / 255.f, (float)lineColor.green() / 255.f, (float)lineColor.blue() / 255.f};
	glUniform3fv(glGetUniformLocation(program, "lineColor"), 1, &lineColorVec3.x);

	if(lineSize != -1.f)	
		glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_INT, nullptr);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	if(needsSave) saveImageOut();
	needsSave = false;
	
}

void GLWidget::regenerate()
{
	// get colors
	auto& list = owningWindow->allColors;
	
	std::vector<std::array<float, 3>> colorsToChooseFrom;
	
	for(size_t itemID = 0; itemID < list.count(); ++itemID)
	{
		auto listItem = list.item(itemID);
		auto color = listItem->backgroundColor();
		
		colorsToChooseFrom.emplace_back(fromRGB(color.red(), color.green(), color.blue()));
	}
	
	if(colorsToChooseFrom.size() == 0)
	{
		colorsToChooseFrom.push_back({1.f, 1.f, 1.f});
	}

	// generate colors
	std::vector<std::array<float, 3>>  colorsData(numVerts);

	std::random_device rd;
	std::mt19937 gen(rd());
	
	gen.seed(++lastSeed);

	std::uniform_int_distribution<> distr(0, colorsToChooseFrom.size() - 1);

	for (std::array<float, 3>& color : colorsData)
	{

		color = colorsToChooseFrom[distr(gen)];
	}

	glBindBuffer(GL_ARRAY_BUFFER, colors);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 3 * numVerts, colorsData.data());
	
	
	
}

void GLWidget::saveImageOut()
{
	
	
	std::vector<GLubyte> imageData;
	imageData.resize(width() * height() * 4);
	
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	
	
    glReadBuffer(GL_FRONT);
	
	
    glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
	
	std::cout << (int)imageData[23] << " HEHEHE" << std::endl;
	
    lodepng::encode(savePath.toStdString(), imageData, width(), height(), LCT_RGBA, 8);
	
	
	
}

