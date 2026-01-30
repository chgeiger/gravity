#ifndef MARKERSETTINGSPANEL_H
#define MARKERSETTINGSPANEL_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class MarkerSettingsPanel : public QWidget {
    Q_OBJECT

public:
    explicit MarkerSettingsPanel(QWidget *parent = nullptr);

signals:
    void generateRequested(int count, float speedMin, float speedMax, float sizeMin, float sizeMax);

private:
    void emitGenerate();

    QLineEdit *countEdit;
    QLineEdit *speedMinEdit;
    QLineEdit *speedMaxEdit;
    QLineEdit *sizeMinEdit;
    QLineEdit *sizeMaxEdit;
    QPushButton *generateButton;
};

#endif // MARKERSETTINGSPANEL_H
