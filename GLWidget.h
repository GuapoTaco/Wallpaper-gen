#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>

#include <glm/glm.hpp>

#include <chrono>

class Window;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
public:
	explicit GLWidget(Window* parent = 0, Qt::WindowFlags f = 0);
	
	virtual void initializeGL() override;
	virtual void paintGL() override;
	virtual bool event(QEvent* event) override;
	
	
	void markForRegeneration();
	void markForColorRefresh();
	void markForSave(const QString& saveDest);
	
	
	float speed = 30.f;
	float lineSize = 1.f;
	
	QColor lineColor = QColor(0, 0, 0);
	
private:
	
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	
	
	
	void regenerate();
	void saveImageOut();
	
	
	
	GLuint program, vertLocs, vertArray, indicies, colors, numElements, numVerts;

	glm::vec3 location = { 0.f, 10.f, 0.f };
	glm::vec3 forwardVector = { 0.f, -1.f, 0.f };
	glm::vec3 upVector = { 0.f, 0.f, 1.f};

	glm::vec3 velocity;
	
	
	
	bool needsRegenerate = false, needsSave = false, needsColorRefresh = false;
	QString savePath;
	
	QTimer time;
	std::chrono::time_point<std::chrono::system_clock> lastTickTime;
	
	bool isMouseDown = false;
	glm::ivec2 lastMousePos;
	
	Window* owningWindow;
	
	int lastSeed = 0;
	
	
};