#include "TaskApp.h"

TaskApp::TaskApp(QWidget* parent) : QMainWindow(parent) {
    // Создаем центральный виджет и устанавливаем его в MainWindow
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Создаем вертикальный layout для центрального виджета
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // Создаем первую кнопку
    QPushButton* button1 = new QPushButton(QString::fromUtf8("Открыть окно редактирования списка задач."), this);
    layout->addWidget(button1);

    // Создаем вторую кнопку
    QPushButton* button2 = new QPushButton(QString::fromUtf8("Выполнить задачу."), this);
    layout->addWidget(button2);

    // Подключаем сигналы кнопок к слотам
    connect(button1, &QPushButton::clicked, this, &TaskApp::openTextFileViewer);
    connect(button2, &QPushButton::clicked, this, &TaskApp::openLineSubstringDialog);
}

TaskApp::~TaskApp() {
    delete centralWidget;
}

void TaskApp::openTextFileViewer() {
    QString filePath = "Textfile.txt";
    TextFileEdit* viewer = new TextFileEdit(filePath);
    viewer->setWindowTitle(QString::fromUtf8("Редактирование задач."));
    viewer->resize(400, 400);
    viewer->show();
}

void TaskApp::openLineSubstringDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("Выполнить задачу"));

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QLineEdit* substringEdit = new QLineEdit(&dialog);
    substringEdit->setPlaceholderText(QString::fromUtf8("Введите название задачи."));
    layout->addWidget(substringEdit);

    QLineEdit* passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setPlaceholderText(QString::fromUtf8("Введите пароль."));
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString substring = substringEdit->text();
        substring = "$$" + substring + "$$";
        QString filePath = "Textfile.txt";

        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QStringList lines = in.readAll().split(SEP);
            file.close();

            bool found = false;
            QString selectedLine;
            for (const QString& line : lines) {
                if (line.contains(substring)) {
                    selectedLine = line;
                    found = true;
                    break;
                }
            }

            if (found) {
                QString password = passwordEdit->text();
                QRegularExpression regex("~~(.*?)~~");
                QRegularExpressionMatchIterator matches = regex.globalMatch(selectedLine);

                bool passwordMatch = false;
                while (matches.hasNext()) {
                    QRegularExpressionMatch match = matches.next();
                    if (match.captured(1) == password) {
                        passwordMatch = true;
                        break;
                    }
                }

                if (passwordMatch) {
                    QRegularExpression substringRegex("\\$\\$(.*?)\\$\\$");
                    QRegularExpressionMatch substringMatch = substringRegex.match(selectedLine);

                    if (substringMatch.hasMatch()) {
                        QString extractedSubstring = substringMatch.captured(1);

                        QDialog lineDialog(this);
                        lineDialog.setWindowTitle(QString::fromUtf8("Содержание задачи."));

                        QVBoxLayout* lineLayout = new QVBoxLayout(&lineDialog);
                        QLabel* lineLabel = new QLabel(selectedLine, &lineDialog);
                        lineLayout->addWidget(lineLabel);

                        QPushButton* readyButton = new QPushButton(QString::fromUtf8("Выполнить."), &lineDialog);
                        lineLayout->addWidget(readyButton);

                        connect(readyButton, &QPushButton::clicked, [&lineDialog, extractedSubstring]() {
                            QDateTime currentTime = QDateTime::currentDateTime();
                            QString timeString = currentTime.toString("yyyy-MM-dd HH:mm:ss");

                            QFile resultsFile("Results.txt");
                            if (resultsFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                                QTextStream out(&resultsFile);
                                out << "Selected line: " << extractedSubstring << " | Time: " << timeString << "\n";
                                resultsFile.close();
                                QMessageBox::information(&lineDialog,
                                    QString::fromUtf8("Успешно."),
                                    QString::fromUtf8("Факт выполнения успешно сохранён!"));
                            }
                            else {
                                QMessageBox::critical(&lineDialog,
                                    QString::fromUtf8("Ошибка"),
                                    QString::fromUtf8("Ошибка сохранения."));
                            }
                            lineDialog.close();
                            });

                        lineDialog.exec();
                    }
                    else {
                        QMessageBox::critical(this,
                            QString::fromUtf8("Ошибка."),
                            QString::fromUtf8("Такая задача не найдена."));
                    }
                }
                else {
                    QMessageBox::critical(this,
                        QString::fromUtf8("Ошибка."),
                        QString::fromUtf8("Неверный пароль."));
                }
            }
            else {
                QMessageBox::critical(this,
                    QString::fromUtf8("Ошибка."),
                    QString::fromUtf8("Задача не найдена."));
            }
        }
        else {
            QMessageBox::critical(this,
                QString::fromUtf8("Ошибка."),
                QString::fromUtf8("Ошибка открытия файла."));
        }
    }
}