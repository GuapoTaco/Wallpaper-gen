#include "Window.h"
#include <QColorDialog>
#include <QFileDialog>

#include <iostream>

Window::Window ( QWidget* parent, Qt::WindowFlags flags ) : QMainWindow ( parent, flags ), speedSlider(Qt::Horizontal), sizeSlider(Qt::Horizontal), widget(this)
{
	
	
	
	setCentralWidget(&windowWidget);
	windowWidget.setLayout(&layout);
	
	sizeSlider.setMinimum(-1);
	sizeSlider.setMaximum(10);
	sizeLabel.setText(QStringLiteral("Line Size:"));
	connect(&sizeSlider, &QSlider::valueChanged, [this]
		{
			widget.lineSize = sizeSlider.value();
		}
	);
	
	regenerateColors.setText(QStringLiteral("Regenerate Random Colors"));
	connect(&regenerateColors, &QPushButton::clicked, [this]
		{
			widget.markForRegeneration();
		}
	);
	
	connect(&allColors, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item)
		{
			QColorDialog* dialog = new QColorDialog;
			dialog->setCurrentColor(item->backgroundColor());
			dialog->show();
			
			connect(dialog, &QColorDialog::currentColorChanged, [this, dialog, item](const QColor& selectedColor)
				{
					
					item->setBackgroundColor(selectedColor);
					
					widget.markForColorRefresh();
				}
			);
		}
	);
	
	newColorButton.setText("+");
	connect(&newColorButton, &QPushButton::clicked, [this]
		{
			QColorDialog* dialog = new QColorDialog;
			dialog->show();
			
			connect(dialog, &QColorDialog::colorSelected, [this, dialog](const QColor& selectedColor)
				{
					
					auto item = new QListWidgetItem();
					allColors.addItem(item);
					item->setBackgroundColor(selectedColor);
					
					widget.markForRegeneration();
				}
			);
		}
	);
	allColorsLabel.setText(QStringLiteral("Colors:"));
	
	deleteColorButton.setText("Remove");
	connect(&deleteColorButton, &QPushButton::clicked, [this]
		{
			for(auto elem : allColors.selectedItems())
			{
				delete elem;
				
				widget.markForRegeneration();
			}
		}
	);
	
	
	save.setText("Save");
	connect(&save, &QPushButton::clicked, [this]
		{
			QFileDialog* dialog = new QFileDialog;
			
            dialog->setAcceptMode(QFileDialog::AcceptSave);
            dialog->setFilter(QDir::Files | QDir::Writable);
            dialog->setNameFilter("*.png");
            dialog->setViewMode(QFileDialog::ViewMode::Detail);


			dialog->open();
			
			connect(dialog, &QFileDialog::fileSelected, [this](const QString& file)
				{
					widget.markForSave(file);
				}
			);
		}
	);
	
	
	layout.addWidget(&widget, 0, 0, 10, 8);
	
	
	// header for colors
	{
		QWidget* encap = new QWidget();
		QHBoxLayout* encapLayout = new QHBoxLayout();
		
		encap->setLayout(encapLayout);
		
		encapLayout->addWidget(&allColorsLabel);
		encapLayout->addWidget(&newColorButton);
		encapLayout->addWidget(&deleteColorButton);
		
		layout.addWidget(encap, 0, 8, 1, 2);
	}
	layout.addWidget(&allColors, 1, 8, 9, 2);
	
	
	layout.addWidget(&sizeLabel, 11, 0, 1, 1);
	layout.addWidget(&sizeSlider, 11, 1, 1, 9);
	
	layout.addWidget(&regenerateColors, 12, 0, 1, 5);
	layout.addWidget(&save, 12, 5, 1, 5);
}

void Window::toggleFullscreen()
{
	if(isGLFullscreen)
	{
		setCentralWidget(&windowWidget);
		isGLFullscreen = false;
	}
	else
	{
		setCentralWidget(&widget);
		isGLFullscreen = true;
	}
}

