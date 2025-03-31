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

class TextFileEdit; // Предварительное объявление

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