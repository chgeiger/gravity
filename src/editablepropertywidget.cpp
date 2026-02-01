#include "editablepropertywidget.h"

#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QDoubleValidator>
#include <QLocale>

EditablePropertyWidget::EditablePropertyWidget(const QString &label, const QString &initialValue, 
                                               QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    labelWidget = new QLabel(label, this);
    labelWidget->setMinimumWidth(100);
    layout->addWidget(labelWidget);

    editField = new QLineEdit(initialValue, this);
    editField->setMaximumWidth(100);
    
    // Nur Dezimalzahlen erlauben
    auto *validator = new QDoubleValidator(0.0, 1000.0, 4, editField);
    validator->setLocale(QLocale::c());
    editField->setValidator(validator);
    
    layout->addWidget(editField);
    layout->addStretch(1);

    connect(editField, &QLineEdit::editingFinished, this, &EditablePropertyWidget::onEditingFinished);
    connect(editField, &QLineEdit::textChanged, this, &EditablePropertyWidget::onTextChanged);
}

QString EditablePropertyWidget::getValue() const
{
    return editField->text();
}

void EditablePropertyWidget::setValue(const QString &value)
{
    editField->blockSignals(true);
    editField->setText(value);
    editField->blockSignals(false);
}

void EditablePropertyWidget::setLabel(const QString &label)
{
    labelWidget->setText(label);
}

void EditablePropertyWidget::setEditable(bool editable)
{
    editField->setReadOnly(!editable);
}

void EditablePropertyWidget::onEditingFinished()
{
    emit editingFinished();
    emit valueChanged(editField->text());
}

void EditablePropertyWidget::onTextChanged(const QString &text)
{
    // Optional: Real-time validation feedback
}
