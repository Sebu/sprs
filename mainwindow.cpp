#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGLWidget>
#include "glwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    GLWidget *seb =  new GLWidget(ui->imageWidget);
    this->connect(ui->pushButton,SIGNAL(clicked()),seb,SLOT(changeImage()));




}

MainWindow::~MainWindow()
{
    delete ui;
}
