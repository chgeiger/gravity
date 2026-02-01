#ifndef EDITABLEPROPERTYWIDGET_H
#define EDITABLEPROPERTYWIDGET_H

#include <QWidget>
#include <QString>

class QLineEdit;
class QLabel;

/**
 * @brief EditablePropertyWidget - Wiederverwendbares Widget zur Bearbeitung von Marker-Eigenschaften
 * 
 * Verantwortlichkeiten:
 * - Anzeige eines Labels und eines editierbaren Textfelds
 * - Validierung der Eingabewerte (Optional)
 * - Emission von Signalen bei Wertaenderungen
 * - Flexible Verwendung fuer verschiedene Marker-Eigenschaften (Dichte, Groesse, etc.)
 */
class EditablePropertyWidget : public QWidget {
    Q_OBJECT

public:
    explicit EditablePropertyWidget(const QString &label, const QString &initialValue = "0.0", 
                                   QWidget *parent = nullptr);

    QString getValue() const;
    void setValue(const QString &value);
    void setLabel(const QString &label);
    void setEditable(bool editable);

signals:
    void valueChanged(const QString &newValue);
    void editingFinished();

private slots:
    void onEditingFinished();
    void onTextChanged(const QString &text);

private:
    QLabel *labelWidget;
    QLineEdit *editField;
};

#endif // EDITABLEPROPERTYWIDGET_H
