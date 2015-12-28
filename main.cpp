#include <QApplication>

#include "Window.h"

int main(int argc, char** argv)
{
	QApplication app{argc, argv};

	Window w;
	//w.setFixedSize(1680, 950);
	
	w.show();
	
	return app.exec();
	
}