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
    void animationToggled(bool running);
    void saveRequested();
    void loadRequested();
    void clearAllMarkersRequested();

private:
    void emitGenerate();

    QLineEdit *countEdit;
    QLineEdit *speedMinEdit;
    QLineEdit *speedMaxEdit;
    QLineEdit *sizeMinEdit;
    QLineEdit *sizeMaxEdit;
    QPushButton *generateButton;
    QPushButton *toggleAnimationButton;
    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *clearButton;
};

#endif // MARKERSETTINGSPANEL_H
