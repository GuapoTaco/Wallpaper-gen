#include <QApplication>

#include "Window.h"

#include <iostream>

int main(int argc, char** argv)
{
	QApplication app{argc, argv};

	Window w;
	//w.setFixedSize(1680, 950);
	
	w.show();
	
    auto ret = app.exec();
	

    std::cin.get();

    return ret;
}
