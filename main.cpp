#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextEdit>
#include <QSlider>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QRegularExpression>

const QString SEP = "`~`&";


class TextFileViewer : public QWidget {
    Q_OBJECT

public:
    TextFileViewer(const QString& filePath, QWidget* parent = nullptr)
        : QWidget(parent), filePath(filePath) {
        loadFile();

        QVBoxLayout* layout = new QVBoxLayout(this);

        // Текстовое поле для отображения и редактирования строки (без строк в ~~)
        textEdit = new QTextEdit(this);
        textEdit->setReadOnly(false); // Разрешаем редактирование
        layout->addWidget(textEdit);

        // Новое текстовое поле для строк, заключенных в ~~
        specialTextEdit = new QTextEdit(this);
        specialTextEdit->setReadOnly(false); // Разрешаем редактирование
        specialTextEdit->setPlaceholderText("Edit password lines (~~text~~) here");
        layout->addWidget(specialTextEdit);

        // Ползунок для выбора строки
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, lines.size() - 1);
        slider->setValue(0);
        layout->addWidget(slider);

        // Кнопка для сохранения изменений
        QPushButton* saveButton = new QPushButton("Save Changes", this);
        layout->addWidget(saveButton);

        // Новая кнопка для добавления нового разделителя
        QPushButton* addSeparatorButton = new QPushButton("Add New Task", this);
        layout->addWidget(addSeparatorButton);

        // Кнопка для удаления текущей строки
        QPushButton* deleteButton = new QPushButton("Delete Current Task", this);
        layout->addWidget(deleteButton);

        // Подключаем сигналы
        connect(slider, &QSlider::valueChanged, this, &TextFileViewer::updateText);
        connect(saveButton, &QPushButton::clicked, this, &TextFileViewer::saveChanges);
        connect(addSeparatorButton, &QPushButton::clicked, this, &TextFileViewer::addNewLine);
        connect(deleteButton, &QPushButton::clicked, this, &TextFileViewer::deleteCurrentLine);

        // Инициализация текста
        updateText(0);
    }

private slots:
    // Обновление текста при изменении ползунка
    void updateText(int value) {
        if (value >= 0 && value < lines.size()) {
            QString currentLine = lines[value];

            // Удаляем строки, заключенные в ~~, для отображения в textEdit
            QRegularExpression regex("~~.*?~~");
            QString displayText = currentLine;
            displayText.remove(regex); // Удаляем все вхождения ~~строка~~
            textEdit->setText(displayText);

            // Ищем строки, заключенные в ~~, и отображаем их в specialTextEdit
            QRegularExpression specialRegex("~~(.*?)~~");
            QRegularExpressionMatchIterator matches = specialRegex.globalMatch(currentLine);

            QStringList specialLines;
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                specialLines.append(match.captured(1)); // Добавляем текст внутри ~~
            }

            // Отображаем найденные строки в specialTextEdit
            specialTextEdit->setText(specialLines.join("\n"));
        }
    }

    // Сохранение изменений в файл
    void saveChanges() {
        int currentLine = slider->value();
        if (currentLine >= 0 && currentLine < lines.size()) {
            // Обновляем строку в списке
            QString updatedLine = textEdit->toPlainText();

            // Обновляем строки, заключенные в ~~, из specialTextEdit
            QStringList specialLines = specialTextEdit->toPlainText().split("\n");
            QRegularExpression regex("~~(.*?)~~");
            QRegularExpressionMatchIterator matches = regex.globalMatch(lines[currentLine]);

            int specialIndex = 0;
            QString newLine = lines[currentLine];
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                if (specialIndex < specialLines.size()) {
                    // Заменяем текст внутри ~~ на новый
                    newLine.replace(match.captured(0), "~~" + specialLines[specialIndex] + "~~");
                    specialIndex++;
                }
            }

            // Обновляем строку
            lines[currentLine] = newLine;

            // Сохраняем все строки в файл
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << lines.join(SEP);
                file.close();
                QMessageBox::information(this, "Success", "Changes saved successfully!");
            }
            else {
                QMessageBox::critical(this, "Error", "Failed to save changes!");
            }
        }
    }

    // Добавление нового разделителя и пустой строки
    void addNewLine() {
        // Проверяем, есть ли строка, заключенная в ~~
        bool hasSpecialLine = false;
        for (const QString& line : lines) {
            if (line.contains("~~")) {
                hasSpecialLine = true;
                break;
            }
        }

        if (!hasSpecialLine) {
            // Если нет строки с ~~, показываем ошибку
            QMessageBox::critical(this, "Error", "No task with password found! Cannot add a new task.");
            return;
        }

        // Добавляем новый разделитель и пустую строку
        lines.append("");
        slider->setRange(0, lines.size() - 1);
        slider->setValue(lines.size() - 1);
        updateText(lines.size() - 1);
    }

    // Удаление текущей строки
    void deleteCurrentLine() {
        int currentLine = slider->value();
        if (currentLine >= 0 && currentLine < lines.size()) {
            // Удаляем текущую строку
            lines.removeAt(currentLine);

            // Обновляем ползунок
            slider->setRange(0, lines.size() - 1);

            // Если строк больше нет, очищаем текстовые поля
            if (lines.isEmpty()) {
                textEdit->clear();
                specialTextEdit->clear();
            }
            else {
                // Переключаемся на предыдущую строку
                if (currentLine >= lines.size()) {
                    currentLine = lines.size() - 1;
                }
                slider->setValue(currentLine);
                updateText(currentLine);
            }

            // Сохраняем изменения в файл
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << lines.join(SEP);
                file.close();
                QMessageBox::information(this, "Success", "Line deleted successfully!");
            }
            else {
                QMessageBox::critical(this, "Error", "Failed to save changes after deletion!");
            }
        }
    }

private:
    // Загрузка строк из файла
    void loadFile() {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            lines = in.readAll().split(SEP);
            file.close();
        }
    }

    QString filePath;
    QStringList lines;
    QTextEdit* textEdit;
    QTextEdit* specialTextEdit; // Новое поле для строк, заключенных в ~~
    QSlider* slider;
};

// Класс главного окна
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        // Создаем центральный виджет и устанавливаем его в MainWindow
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // Создаем вертикальный layout для центрального виджета
        QVBoxLayout* layout = new QVBoxLayout(centralWidget);

        // Создаем первую кнопку
        QPushButton* button1 = new QPushButton("Open Create Tasks Window", this);
        layout->addWidget(button1);

        // Создаем вторую кнопку
        QPushButton* button2 = new QPushButton("Select Task by Number", this);
        layout->addWidget(button2);

        // Подключаем сигналы кнопок к слотам
        connect(button1, &QPushButton::clicked, this, &MainWindow::openTextFileViewer);
        connect(button2, &QPushButton::clicked, this, &MainWindow::openLineNumberDialog);
    }

private slots:
    void openTextFileViewer() {
        // Укажите путь к вашему текстовому файлу
        QString filePath = "Textfile.txt";

        // Создаем и показываем окно TextFileViewer
        TextFileViewer* viewer = new TextFileViewer(filePath);
        viewer->setWindowTitle("Create Tasks");
        viewer->resize(400, 400); // Увеличиваем размер окна для нового поля
        viewer->show();
    }

    void openLineNumberDialog() {
        // Создаем диалоговое окно
        QDialog dialog(this);
        dialog.setWindowTitle("Select Task Number");

        // Создаем layout для диалога
        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        // Поле для ввода номера строки
        QLineEdit* lineEdit = new QLineEdit(&dialog);
        lineEdit->setPlaceholderText("Enter task number");
        layout->addWidget(lineEdit);

        // Поле для ввода пароля
        QLineEdit* passwordEdit = new QLineEdit(&dialog);
        passwordEdit->setPlaceholderText("Enter password");
        passwordEdit->setEchoMode(QLineEdit::Password); // Скрываем ввод пароля
        layout->addWidget(passwordEdit);

        // Кнопки подтверждения и отмены
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttonBox);

        // Подключаем кнопки к слотам
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        // Показываем диалог и обрабатываем результат
        if (dialog.exec() == QDialog::Accepted) {
            bool ok;
            int lineNumber = lineEdit->text().toInt(&ok);

            if (ok) {
                // Укажите путь к вашему текстовому файлу
                QString filePath = "Textfile.txt";

                // Загружаем файл
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    QStringList lines = in.readAll().split(SEP);
                    file.close();

                    // Проверяем, существует ли строка с таким номером
                    if (lineNumber >= 1 && lineNumber <= lines.size()) {
                        QString selectedLine = lines[lineNumber - 1];

                        // Проверяем, совпадает ли введенный пароль с любой строкой в ~~
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
                            // Отображаем текст строки
                            QMessageBox::information(this, "Task Text", selectedLine);
                        }
                        else {
                            // Сообщение об ошибке
                            QMessageBox::critical(this, "Error", "Incorrect password!");
                        }
                    }
                    else {
                        // Сообщение об ошибке
                        QMessageBox::critical(this, "Error", "Invalid task number!");
                    }
                }
                else {
                    QMessageBox::critical(this, "Error", "Failed to open the file!");
                }
            }
            else {
                QMessageBox::critical(this, "Error", "Invalid input!");
            }
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Создаем и показываем MainWindow
    MainWindow mainWindow;
    mainWindow.setWindowTitle("Main Window");
    mainWindow.resize(300, 200);
    mainWindow.show();

    return app.exec();
}

#include "main.moc"