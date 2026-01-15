#include "MainWindow.h"
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt Application");
    resize(800, 600);

    QLabel *label = new QLabel("Hello, Qt!", this);
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
}

MainWindow::~MainWindow()
{
}
