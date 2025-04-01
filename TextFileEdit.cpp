#include "TaskApp.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QMessageBox>

TextFileEdit::TextFileEdit(const QString& filePath, QWidget* parent)
    : QWidget(parent), filePath(filePath)
{
    NewLoad(filePath);  
    setupUI();
}

void TextFileEdit::NewLoad(const QString& filePath) {
    DataFile file(filePath);
    QString content;

    if (!file.loadFile(content)) {
        QMessageBox::critical(this,
            QString::fromUtf8("Ошибка"),
            QString::fromUtf8("Не удалось открыть файл!"));
        return;
    }

    // Очищаем текущие данные
    items.clear();
    titles.clear();

    // Разбиваем содержимое на пары с помощью DataFile
    QList<QStringList> nameAndText = file.splitContentToPairs(content, SEP, SEPNAME);

    // Обрабатываем каждую пару
    for (const QStringList& pair : nameAndText) {
        if (pair.size() >= 2) {
            QString item = pair[1]; // Основной текст
            items.append(item);

            // Извлекаем название
            extractTitle(item);

            // Обновляем последний элемент в items (так как extractTitle модифицирует строку)
            if (!items.isEmpty()) {
                items[items.size() - 1] = item;
            }
        }
    }

    // Обновляем UI, если он уже был создан
    if (slider) {
        slider->setRange(0, items.size() - 1);
        if (!items.isEmpty()) {
            slider->setValue(0);
            updateTextEdit(0);
        }
    }
}
void TextFileEdit::extractTitle(QString& item) {
    // Извлечение названия (подстрока между $$)
    QRegularExpression titleRegex("\\$\\$(.*?)\\$\\$");
    QRegularExpressionMatch titleMatch = titleRegex.match(item);
    if (titleMatch.hasMatch()) {
        titles.append(titleMatch.captured(1));
        item.remove(titleMatch.captured(0));
    }
    else {
        titles.append("");
    }
}


void TextFileEdit::updateTextEdit(int index) {
    if (index >= 0 && index < items.size()) {
        textEdit->setText(items[index]);
        titleEdit->setText(titles[index]);
        updateTimeAndCount(titles[index]);
    }
}

void TextFileEdit::updateTimeAndCount(const QString& title) {
    QFile resultsFile("DataResults.txt");
    if (resultsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&resultsFile);
        QString lastTime;
        int count = 0;

        QRegularExpression regex(QString("Selected line: %1 \\| Time: (\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2})").arg(title));

        while (!in.atEnd()) {
            QString line = in.readLine();
            QRegularExpressionMatch match = regex.match(line);
            if (match.hasMatch()) {
                lastTime = match.captured(1);
                count++;
            }
        }

        resultsFile.close();

        lastTimeEdit->setText(lastTime.isEmpty() ? QString::fromUtf8("Нет данных") : lastTime);
        countEdit->setText(QString::number(count));
    }
    else {
        lastTimeEdit->setText(QString::fromUtf8("Ошибка чтения файла"));
        countEdit->setText(QString::fromUtf8("Ошибка чтения файла"));
    }
}

void TextFileEdit::saveChanges() {
    int index = slider->value();
    if (index >= 0 && index < items.size()) {
        items[index] = textEdit->toPlainText();
        titles[index] = titleEdit->text();
        saveFile();

        QMessageBox::information(this,
            QString::fromUtf8("Сохранено"),
            QString::fromUtf8("Изменения успешно сохранены!"));
    }
}

void TextFileEdit::saveFile() {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        for (int i = 0; i < items.size(); ++i) {
            QString item = items[i];
            QString title = titles[i];

            if (!title.isEmpty()) {
                item.prepend("$$" + title + "$$ ");
            }

            out << item;
            if (i < items.size() - 1) {
                out << SEP;
            }
        }

        file.close();
    }
    else {
        QMessageBox::critical(this,
            QString::fromUtf8("Ошибка"),
            QString::fromUtf8("Не удалось открыть файл для записи!"));
    }
}

