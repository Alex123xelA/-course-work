#include "TaskApp.h"

void TextFileEdit::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Поле для отображения названия
    titleEdit = new QLineEdit(this);
    layout->addWidget(titleEdit);

    // Основное текстовое поле
    textEdit = new QTextEdit(this);
    layout->addWidget(textEdit);

    // Поле для отображения последнего времени
    lastTimeEdit = new QLineEdit(this);
    lastTimeEdit->setReadOnly(true);
    layout->addWidget(lastTimeEdit);

    // Поле для отображения количества выполнений
    countEdit = new QLineEdit(this);
    countEdit->setReadOnly(true);
    layout->addWidget(countEdit);

    // Ползунок
    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, items.size() - 1);
    slider->setValue(0);
    layout->addWidget(slider);

    // Кнопки
    QPushButton* saveButton = new QPushButton(QString::fromUtf8("Сохранить"), this);
    QPushButton* addButton = new QPushButton(QString::fromUtf8("Добавить задачу"), this);
    QPushButton* deleteButton = new QPushButton(QString::fromUtf8("Удалить задачу"), this);

    layout->addWidget(saveButton);
    layout->addWidget(addButton);
    layout->addWidget(deleteButton);

    // Отображение первой задачи
    updateTextEdit(0);

    // Соединение сигналов и слотов
    connect(slider, &QSlider::valueChanged, this, &TextFileEdit::updateTextEdit);
    connect(saveButton, &QPushButton::clicked, this, &TextFileEdit::saveChanges);
    connect(addButton, &QPushButton::clicked, this, &TextFileEdit::addNewElement);
    connect(deleteButton, &QPushButton::clicked, this, &TextFileEdit::deleteCurrentElement);
}
