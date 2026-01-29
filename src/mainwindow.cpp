#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Gravity Application");
    setGeometry(100, 100, 400, 300);

    // Zentrales Widget
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Label
    label = new QLabel("Willkommen in der Gravity Anwendung!", this);
    layout->addWidget(label);

    // Button
    button = new QPushButton("Klick mich!", this);
    connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    layout->addWidget(button);

    layout->addStretch();
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

void MainWindow::onButtonClicked() {
    static int counter = 0;
    counter++;
    label->setText("Button wurde " + QString::number(counter) + " mal geklickt!");
}
