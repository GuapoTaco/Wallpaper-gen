#include "Window.h"
#include <QColorDialog>

Window::Window ( QWidget* parent, Qt::WindowFlags flags ) : QMainWindow ( parent, flags ), speedSlider(Qt::Horizontal), sizeSlider(Qt::Horizontal)
{
	setCentralWidget(&windowWidget);
	windowWidget.setLayout(&layout);
	
	speedSlider.setMinimum(-1);
	speedSlider.setMaximum(100);
	speedLabel.setText(QStringLiteral("Speed:"));
	connect(&speedSlider, &QSlider::valueChanged, [this]
		{
			widget.speed = speedSlider.value();
		}
	);
	
	sizeSlider.setMinimum(0);
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
	
	newColorButton.setText("+");
	connect(&newColorButton, &QPushButton::clicked, [this]
		{
			QColorDialog* dialog = new QColorDialog;
			dialog->show();
			
			connect(dialog, &QColorDialog::colorSelected, [this, dialog]
				{
					
					colorStorage.push_back(QListWidgetItem());
					auto& item = colorStorage[colorStorage.size() - 1];
					item.setBackgroundColor(dialog->currentColor());
					allColors.insertItem(allColors.count() + 1, &item);
					
				}
			);
		}
	);
	allColorsLabel.setText(QStringLiteral("Colors:"));
	
	
	layout.addWidget(&widget, 0, 0, 10, 8);
	
	layout.addWidget(&allColorsLabel, 0, 8, 1, 1);
	layout.addWidget(&newColorButton, 0, 9, 1, 1);
	layout.addWidget(&allColors, 1, 8, 9, 2);
	
	layout.addWidget(&speedLabel, 11, 0, 1, 1);
	layout.addWidget(&speedSlider, 11, 1, 1, 9);
	
	layout.addWidget(&sizeLabel, 12, 0, 1, 1);
	layout.addWidget(&sizeSlider, 12, 1, 1, 9);
	
	layout.addWidget(&regenerateColors, 13, 0, 1, 10);
}
