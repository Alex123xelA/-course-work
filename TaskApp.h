#pragma once

#include <QtWidgets/QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include <QDialogButtonBox>
#include <QDialog>
#include <QRegularExpressionMatchIterator>
#include <QLabel>
#include <QMessageBox>
#include <QTextEdit>
#include <QSlider>
#include <QDateTime>

//class TextFileEdit; // Предварительное объявление

const QString SEP = "`~`&";

class TaskApp : public QMainWindow
{
    Q_OBJECT

public:
    TaskApp(QWidget* parent = nullptr);
    ~TaskApp();

private slots:
    void openTextFileViewer();
    void openLineSubstringDialog();

private:
    QWidget* centralWidget;
};

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