#include "taskApp.h"

void TextFileEdit::addNewElement() {
    items.append(QString::fromUtf8("Новый элемент"));
    titles.append(QString::fromUtf8("Новое название"));

    slider->setRange(0, items.size() - 1);
    slider->setValue(items.size() - 1);
    saveFile();

    QMessageBox::information(this,
        QString::fromUtf8("Добавлено"),
        QString::fromUtf8("Новая задача успешно добавлена!"));
}
