#include "TextFileEdit.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QMessageBox>

TextFileEdit::TextFileEdit(const QString& filePath, QWidget* parent)
    : QWidget(parent), filePath(filePath)
{
    loadFile(filePath);
    setupUI();
}

void TextFileEdit::loadFile(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        items = content.split(SEP, Qt::SkipEmptyParts);

        for (QString& item : items) {
            extractPasswordAndTitle(item);
        }
    }
}

void TextFileEdit::extractPasswordAndTitle(QString& item) {
    // Извлечение пароля (подстрока между ~~)
    QRegularExpression passwordRegex("~~(.*?)~~");
    QRegularExpressionMatch passwordMatch = passwordRegex.match(item);
    if (passwordMatch.hasMatch()) {
        passwords.append(passwordMatch.captured(1));
        item.remove(passwordMatch.captured(0));
    }
    else {
        passwords.append("");
    }

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

void TextFileEdit::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Поле для отображения названия
    titleEdit = new QLineEdit(this);
    layout->addWidget(titleEdit);

    // Поле для отображения пароля
    passwordEdit = new QLineEdit(this);
    layout->addWidget(passwordEdit);

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

void TextFileEdit::updateTextEdit(int index) {
    if (index >= 0 && index < items.size()) {
        textEdit->setText(items[index]);
        titleEdit->setText(titles[index]);
        passwordEdit->setText(passwords[index]);
        updateTimeAndCount(titles[index]);
    }
}

void TextFileEdit::updateTimeAndCount(const QString& title) {
    QFile resultsFile("Results.txt");
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
        passwords[index] = passwordEdit->text();
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
            QString password = passwords[i];

            if (!title.isEmpty()) {
                item.prepend("$$" + title + "$$ ");
            }
            if (!password.isEmpty()) {
                item.prepend("~~" + password + "~~ ");
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

void TextFileEdit::addNewElement() {
    items.append(QString::fromUtf8("Новый элемент"));
    titles.append(QString::fromUtf8("Новое название"));
    passwords.append(QString::fromUtf8("Новый пароль"));

    slider->setRange(0, items.size() - 1);
    slider->setValue(items.size() - 1);
    saveFile();

    QMessageBox::information(this,
        QString::fromUtf8("Добавлено"),
        QString::fromUtf8("Новая задача успешно добавлена!"));
}

void TextFileEdit::deleteCurrentElement() {
    int index = slider->value();
    if (index >= 0 && index < items.size()) {
        items.removeAt(index);
        titles.removeAt(index);
        passwords.removeAt(index);

        slider->setRange(0, items.size() - 1);
        if (items.isEmpty()) {
            textEdit->clear();
            titleEdit->clear();
            passwordEdit->clear();
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