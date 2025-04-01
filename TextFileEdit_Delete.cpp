#include "TaskApp.h"
void TextFileEdit::deleteCurrentElement() {
    int index = slider->value();
    if (index >= 0 && index < items.size()) {
        items.removeAt(index);
        titles.removeAt(index);

        slider->setRange(0, items.size() - 1);
        if (items.isEmpty()) {
            textEdit->clear();
            titleEdit->clear();
            lastTimeEdit->clear();
            countEdit->clear();
        }
        else {
            slider->setValue(qMax(0, index - 1));
        }

        saveFile();

        QMessageBox::information(this,
            QString::fromUtf8("Удалено"),
            QString::fromUtf8("Задача успешно удалена!"));
    }
}