#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QSlider>
#include <QStringList>

const QString SEP = "`~`&";

class TextFileEdit : public QWidget {
    Q_OBJECT

public:
    explicit TextFileEdit(const QString& filePath, QWidget* parent = nullptr);

private slots:
    void updateTextEdit(int index);
    void saveChanges();
    void addNewElement();
    void deleteCurrentElement();

private:
    void loadFile(const QString& filePath);
    void setupUI();
    void updateTimeAndCount(const QString& title);
    void saveFile();
    void extractPasswordAndTitle(QString& item);

    QTextEdit* textEdit;
    QLineEdit* titleEdit;
    QLineEdit* passwordEdit;
    QLineEdit* lastTimeEdit;
    QLineEdit* countEdit;
    QSlider* slider;
    QStringList items;
    QStringList titles;
    QStringList passwords;
    QString filePath;
};