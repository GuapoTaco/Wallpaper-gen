#include <QMainWindow>
#include <QGridLayout>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>

#include <vector>

#include "GLWidget.h"

class Window : public QMainWindow
{
public:
	explicit Window(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	
	QWidget windowWidget;
	QGridLayout layout;
	QLabel speedLabel;
	QSlider speedSlider;
	GLWidget widget;
	
	QSlider sizeSlider;
	QLabel sizeLabel;
	
	QLabel allColorsLabel;
	QListWidget allColors;
	QPushButton newColorButton;
	std::vector<QListWidgetItem> colorStorage;
	
	
	QPushButton regenerateColors;
	
};