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
#include <QFile>

//class TextFileEdit; // Предварительное объявление

const QString SEP = "`~`&";
const QString SEPNAME = "$$";

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
 
    void setupUI();
    void updateTimeAndCount(const QString& title);
    void saveFile();
    void extractTitle(QString& item);
    void NewLoad(const QString& filePath);

    QTextEdit* textEdit;
    QLineEdit* titleEdit;
    QLineEdit* lastTimeEdit;
    QLineEdit* countEdit;
    QSlider* slider;
    QStringList items;
    QStringList titles;
    QString filePath;
};

class DataFile : public QFile {
public:
    DataFile(const QString& filePath, QObject* parent = nullptr);
    bool loadFile(QString& content);
    QList<QStringList> splitContentToPairs(const QString& content,
        const QString& SEP,
        const QString& SEPNAME) const;

private:
    // Дополнительные приватные члены класса
};
