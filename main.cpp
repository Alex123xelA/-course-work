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
#include <QDateTime>

const QString SEP = "`~`&";

class TextFileViewer : public QWidget {
    Q_OBJECT

public:
    TextFileViewer(const QString& filePath, const QString& resultsFilePath, const QString& separator, QWidget* parent = nullptr)
        : QWidget(parent), filePath(filePath), resultsFilePath(resultsFilePath), separator(separator) {
        // Загрузка файла
        if (!loadFile()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось загрузить файл.");
            return;
        }

        // Создание интерфейса
        setupUI();
    }

private:
    bool loadFile() {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false; // Не удалось открыть файл
        }

        QTextStream in(&file);
        QString content = in.readAll(); // Чтение файла
        file.close();

        // Разделение содержимого
        lines = content.split(separator, Qt::SkipEmptyParts);

        return true;
    }

    void setupUI() {
        QVBoxLayout* layout = new QVBoxLayout(this);

        // Текстовое поле для отображения текущей строки (без подстроки в ~~)
        textEdit = new QTextEdit(this);
        textEdit->setReadOnly(false); // Разрешаем редактирование
        layout->addWidget(textEdit);

        // Текстовое поле для отображения и редактирования подстроки, заключенной в ~~
        substringEdit = new QLineEdit(this);
        substringEdit->setPlaceholderText("Введите пароль...");
        layout->addWidget(substringEdit);

        // Текстовое поле для отображения времени редактирования задачи
        timeLabel = new QLabel(this);
        timeLabel->setAlignment(Qt::AlignCenter);
        timeLabel->setStyleSheet("font-weight: bold; color: green;"); 
        layout->addWidget(timeLabel);

        // Текстовое поле для отображения количества выполнений задачи
        saveCountLabel = new QLabel(this);
        saveCountLabel->setAlignment(Qt::AlignCenter);
        saveCountLabel->setStyleSheet("font-weight: bold; color: blue;"); 
        layout->addWidget(saveCountLabel);

        // Кнопка для сохранения изменений
        QPushButton* saveButton = new QPushButton("Сохранить изменения", this);
        layout->addWidget(saveButton);

        // Кнопка для добавления новой задачи
        QPushButton* addButton = new QPushButton("Добавить новую задачу", this);
        layout->addWidget(addButton);

        // Кнопка для удаления текущей задачи
        QPushButton* deleteButton = new QPushButton("Удалить текущую задачу", this);
        layout->addWidget(deleteButton);

        // Кнопка для очистки файла Results.txt
        QPushButton* clearResultsButton = new QPushButton("Отменить выполнение задач", this);
        layout->addWidget(clearResultsButton);

        // Ползунок
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, lines.size() - 1);
        slider->setValue(0);
        layout->addWidget(slider);

        // Обновление текстовых полей при изменении ползунка
        connect(slider, &QSlider::valueChanged, this, &TextFileViewer::updateText);

        // Сохранение изменений при нажатии на кнопку
        connect(saveButton, &QPushButton::clicked, this, &TextFileViewer::saveChanges);

        // Добавление новой задачи при нажатии на кнопку
        connect(addButton, &QPushButton::clicked, this, &TextFileViewer::addNewLine);

        // Удаление текущей задачи при нажатии на кнопку
        connect(deleteButton, &QPushButton::clicked, this, &TextFileViewer::deleteCurrentLine);

        // Очистка файла Results.txt при нажатии на кнопку
        connect(clearResultsButton, &QPushButton::clicked, this, &TextFileViewer::clearResultsFile);

        // Инициализация текста
        updateText(0);

        setLayout(layout);
    }

private slots:
    void updateText(int value) {
        if (value >= 0 && value < lines.size()) {
            currentLineIndex = value; // Сохраняем индекс текущей задачи

            // Установка текущей задачи в текстовое поле (без пароля)
            QString currentLine = lines[currentLineIndex];
            QString displayText = removeSubstring(currentLine); 
            textEdit->setText(displayText);

            // Поиск пароля
            QString substring = extractSubstring(currentLine);
            substringEdit->setText(substring);

            // Обновление времени последнего выполнения задачи
            updateTimeLabel(currentLineIndex + 1); 

            // Обновление количества сохранений строки
            updateSaveCountLabel(currentLineIndex + 1); 
        }
    }

    void saveChanges() {
        if (currentLineIndex >= 0 && currentLineIndex < lines.size()) {
            // Обновляем текущую задачу с учетом изменений в пароле
            QString newLine = textEdit->toPlainText();
            QString newSubstring = substringEdit->text();

            // Восстанавливаем пароль
            QString updatedLine = newLine + "~~" + newSubstring + "~~";
            lines[currentLineIndex] = updatedLine;

            // Обновляем текстовое поле (без пароля)
            textEdit->setText(newLine);

            // Сохраняем изменения в файл
            saveFile();

            // Записываем факт сохранения в Results.txt
            logSaveEvent(currentLineIndex + 1); 

            // Обновляем количество выполнений
            updateSaveCountLabel(currentLineIndex + 1);
        }
    }

    void addNewLine() {
        // Добавляем новую задачу в конец списка
        lines.append("~~~~"); // Пустой пароль

        // Обновляем диапазон ползунка
        slider->setRange(0, lines.size() - 1);

        // Переключаемся на новую строку
        currentLineIndex = lines.size() - 1;
        slider->setValue(currentLineIndex);

        // Обновляем текстовые поля
        updateText(currentLineIndex);

        // Сохраняем изменения в файл
        saveFile();
    }

    void deleteCurrentLine() {
        if (currentLineIndex >= 0 && currentLineIndex < lines.size()) {
            // Удаляем текущую задачу
            lines.removeAt(currentLineIndex);

            // Обновляем диапазон ползунка
            slider->setRange(0, lines.size() - 1);

            // Переключаемся на предыдущую задачу, если она есть
            if (currentLineIndex >= lines.size()) {
                currentLineIndex = lines.size() - 1;
            }

            // Обновляем текстовые поля
            updateText(currentLineIndex);

            // Сохраняем изменения в файл
            saveFile();
        }
    }

    // Метод для очистки файла Results.txt
    void clearResultsFile() {
        QFile resultsFile(resultsFilePath);
        if (resultsFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            resultsFile.close();
            QMessageBox::information(this, "Успех", "Выполнение всех задач отменено.");
            updateTimeLabel(currentLineIndex + 1);
            updateSaveCountLabel(currentLineIndex + 1); 
        }
        else {
            QMessageBox::warning(this, "Ошибка", "Не удалось отменить выполнение задач.");
        }
    }
    void updateTimeLabel(int lineNumber) {
        QFile resultsFile(resultsFilePath);
        if (resultsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&resultsFile);
            QString lastTime = "Время не найдено"; 

            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.contains("Selected line number: " + QString::number(lineNumber))) {
                    // Извлекаем время из строки
                    int timePos = line.indexOf("Time: ");
                    if (timePos != -1) {
                        lastTime = line.mid(timePos + 6); 
                    }
                }
            }

            resultsFile.close();
            timeLabel->setText("Последнее открытие: " + lastTime);
        }
        else {
            timeLabel->setText("Не удалось открыть файл Results.txt");
        }
    }
    void updateSaveCountLabel(int lineNumber) {
        QFile resultsFile(resultsFilePath);
        if (resultsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&resultsFile);
            int saveCount = 0;

            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.contains("Selected line number: " + QString::number(lineNumber))) {
                    saveCount++;
                }
            }

            resultsFile.close();
            saveCountLabel->setText("Количество выполнений этой задачи: " + QString::number(saveCount));
        }
        else {
            saveCountLabel->setText("Не удалось получить список выполненых задач.");
        }
    }
    void logSaveEvent(int lineNumber) {
        QFile resultsFile(resultsFilePath);
        if (resultsFile.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&resultsFile);
            out << "Selected line number: " << lineNumber << " | Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
            resultsFile.close();
        }
        else {
            QMessageBox::warning(this, "Ошибка", "Не удалось записать список выполненную задачу.");
        }
    }
    QString extractSubstring(const QString& text) {
        int startPos = text.indexOf("~~");
        int endPos = text.lastIndexOf("~~");

        if (startPos != -1 && endPos != -1 && startPos < endPos) {
            return text.mid(startPos + 2, endPos - startPos - 2); // Извлекаем пароль
        }
        return QString(); // Если пароль не найден
    }
    QString removeSubstring(const QString& text) {
        int startPos = text.indexOf("~~");
        int endPos = text.lastIndexOf("~~");

        if (startPos != -1 && endPos != -1 && startPos < endPos) {
            QString before = text.left(startPos);
            QString after = text.mid(endPos + 2);
            return before + after; // Удаляем пароль
        }
        return text;
    }
    void saveFile() {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << lines.join(separator); 
            file.close();
        }
        else {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить изменения.");
        }
    }

private:
    QString filePath;
    QString resultsFilePath;
    QString separator;
    QStringList lines;
    QTextEdit* textEdit;
    QLineEdit* substringEdit;
    QLabel* timeLabel;
    QLabel* saveCountLabel;
    QSlider* slider;
    int currentLineIndex = -1; // Индекс текущей задачи
};
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
        QPushButton* button1 = new QPushButton("Создать и редактировать задачи.", this);
        layout->addWidget(button1);

        // Создаем вторую кнопку
        QPushButton* button2 = new QPushButton("Выполнять задачи.", this);
        layout->addWidget(button2);

        // Подключаем сигналы кнопок к слотам
        connect(button1, &QPushButton::clicked, this, &MainWindow::openTextFileViewer);
        connect(button2, &QPushButton::clicked, this, &MainWindow::openLineNumberDialog);
    }

private slots:
    void openTextFileViewer() {
        QString filePath = "Textfile.txt";

        // Создаем и показываем окно TextFileViewer
        TextFileViewer* viewer = new TextFileViewer(filePath, "Results.txt", SEP);
        viewer->setWindowTitle("Создание и редактирование задач");
        viewer->resize(400, 400); // Увеличиваем размер окна для нового поля
        viewer->show();
    }

    void openLineNumberDialog() {
        // Создаем диалоговое окно
        QDialog dialog(this);
        dialog.setWindowTitle("Вход в задачу.");

        // Создаем layout для диалога
        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        // Поле для ввода номера строки
        QLineEdit* lineEdit = new QLineEdit(&dialog);
        lineEdit->setPlaceholderText("Введите номер задачи");
        layout->addWidget(lineEdit);

        // Поле для ввода пароля
        QLineEdit* passwordEdit = new QLineEdit(&dialog);
        passwordEdit->setPlaceholderText("Введите пароль");
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
                            // Создаем новое диалоговое окно для отображения задачи
                            QDialog lineDialog(this);
                            lineDialog.setWindowTitle("Задача:");

                            QVBoxLayout* lineLayout = new QVBoxLayout(&lineDialog);

                            // Отображаем текст задачи
                            QLabel* lineLabel = new QLabel(selectedLine, &lineDialog);
                            lineLayout->addWidget(lineLabel);

                            // Кнопка для записи номера задачи в файл
                            QPushButton* readyButton = new QPushButton("Выполнить.", &lineDialog);
                            lineLayout->addWidget(readyButton);

                            // Подключаем кнопку к слоту
                            connect(readyButton, &QPushButton::clicked, [&lineDialog, lineNumber]() {
                                // Получаем текущее время
                                QDateTime currentTime = QDateTime::currentDateTime();
                                QString timeString = currentTime.toString("yyyy-MM-dd HH:mm:ss");

                                // Записываем номер задачи и время в файл Results.txt
                                QFile resultsFile("Results.txt");
                                if (resultsFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                                    QTextStream out(&resultsFile);
                                    out << "Selected line number: " << lineNumber << " | Time: " << timeString << "\n";
                                    resultsFile.close();
                                    QMessageBox::information(&lineDialog, "Выполнено", "Факт выполнения задачи зафиксирован!");
                                }
                                else {
                                    QMessageBox::critical(&lineDialog, "Ошибка", "Факт выполнения задачи не зафиксирован.");
                                }
                                lineDialog.close();
                                });

                            // Показываем диалог
                            lineDialog.exec();
                        }
                        else {
                            // Сообщение об ошибке
                            QMessageBox::critical(this, "Ошибка", "Неправильный пароль.");
                        }
                    }
                    else {
                        // Сообщение об ошибке
                        QMessageBox::critical(this, "Ошибка", "Неправильный номер задачи.");
                    }
                }
                else {
                    QMessageBox::critical(this, "Ошибка", "Ошибка открытия файла.");
                }
            }
            else {
                QMessageBox::critical(this, "Ошибка", "Неправильный ввод.");
            }
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.setWindowTitle("Выбор режима.");
    mainWindow.resize(300, 200);
    mainWindow.show();

    return app.exec();
}

#include "main.moc"