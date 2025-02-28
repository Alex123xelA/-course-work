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
    TextFileViewer(const QString& filePath, QWidget* parent = nullptr)
        : QWidget(parent), filePath(filePath) {
        loadFile(filePath);
        // Создание интерфейса
        setupUI();
    }

private:
    void loadFile(const QString& filePath) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();

            // Разделение содержимого файла по разделителю SEP
            items = content.split(SEP, Qt::SkipEmptyParts);

            // Обработка каждого элемента для извлечения пароля и названия
            for (QString& item : items) {
                extractPasswordAndTitle(item);
            }
        }
    }

    void extractPasswordAndTitle(QString& item) {
        // Извлечение пароля (подстрока между ~~)
        QRegularExpression passwordRegex("~~(.*?)~~");
        QRegularExpressionMatch passwordMatch = passwordRegex.match(item);
        if (passwordMatch.hasMatch()) {
            passwords.append(passwordMatch.captured(1)); // Сохраняем пароль
            item.remove(passwordMatch.captured(0)); // Удаляем пароль из основного текста
        }
        else {
            passwords.append(""); // Если пароль не найден
        }

        // Извлечение названия (подстрока между $$)
        QRegularExpression titleRegex("\\$\\$(.*?)\\$\\$");
        QRegularExpressionMatch titleMatch = titleRegex.match(item);
        if (titleMatch.hasMatch()) {
            titles.append(titleMatch.captured(1)); // Сохраняем название
            item.remove(titleMatch.captured(0)); // Удаляем название из основного текста
        }
        else {
            titles.append(""); // Если название не найдено
        }
    }

    void setupUI() {
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

        // Кнопка "Сохранить"
        QPushButton* saveButton = new QPushButton("Сохранить", this);
        layout->addWidget(saveButton);

        // Кнопка "Добавить задачу"
        QPushButton* addButton = new QPushButton("Добавить задачу", this);
        layout->addWidget(addButton);

        // Кнопка "Удалить задачу"
        QPushButton* deleteButton = new QPushButton("Удалить задачу", this);
        layout->addWidget(deleteButton);

        // Отображение первой задачи
        updateTextEdit(0);

        // Соединение сигнала изменения значения ползунка с обновлением текстового поля
        connect(slider, &QSlider::valueChanged, this, &TextFileViewer::updateTextEdit);

        // Соединение кнопки "Сохранить" с методом сохранения
        connect(saveButton, &QPushButton::clicked, this, &TextFileViewer::saveChanges);

        // Соединение кнопки "Добавить элемент" с методом добавления нового элемента
        connect(addButton, &QPushButton::clicked, this, &TextFileViewer::addNewElement);

        // Соединение кнопки "Удалить элемент" с методом удаления текущего элемента
        connect(deleteButton, &QPushButton::clicked, this, &TextFileViewer::deleteCurrentElement);
    }

    void updateTextEdit(int index) {
        if (index >= 0 && index < items.size()) {
            // Обновление основного текстового поля
            textEdit->setText(items[index]);

            // Обновление поля с названием
            titleEdit->setText(titles[index]);

            // Обновление поля с паролем
            passwordEdit->setText(passwords[index]);

            // Обновление данных о времени и количестве строк
            updateTimeAndCount(titles[index]);
        }
    }

    void updateTimeAndCount(const QString& title) {
        QFile resultsFile("Results.txt");
        if (resultsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&resultsFile);
            QString lastTime;
            int count = 0;

            // Поиск данных для отображения
            QRegularExpression regex(QString("Selected line: %1 \\| Time: (\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2})").arg(title));

            while (!in.atEnd()) {
                QString line = in.readLine();
                QRegularExpressionMatch match = regex.match(line);
                if (match.hasMatch()) {
                    lastTime = match.captured(1); // Последнее время
                    count++;
                }
            }

            resultsFile.close();

            // Обновление полей
            lastTimeEdit->setText(lastTime.isEmpty() ? "Нет данных" : lastTime);
            countEdit->setText(QString::number(count));
        }
        else {
            lastTimeEdit->setText("Ошибка чтения файла");
            countEdit->setText("Ошибка чтения файла");
        }
    }

    void saveChanges() {
        int index = slider->value();
        if (index >= 0 && index < items.size()) {
            // Обновление данных
            items[index] = textEdit->toPlainText();
            titles[index] = titleEdit->text();
            passwords[index] = passwordEdit->text();

            // Сохранение данных в файл
            saveFile();

            QMessageBox::information(this, "Сохранено", "Изменения успешно сохранены!");
        }
    }

    void addNewElement() {
        // Добавление новой задачи
        items.append("Новый элемент");
        titles.append("Новое название");
        passwords.append("Новый пароль");

        // Обновление диапазона ползунка
        slider->setRange(0, items.size() - 1);

        // Переход к новой задаче
        slider->setValue(items.size() - 1);

        // Сохранение изменений в файл
        saveFile();

        QMessageBox::information(this, "Добавлено", "Новая задача успешно добавлена!");
    }

    void deleteCurrentElement() {
        int index = slider->value();
        if (index >= 0 && index < items.size()) {
            // Удаление текущей задачи
            items.removeAt(index);
            titles.removeAt(index);
            passwords.removeAt(index);

            // Обновление диапазона ползунка
            slider->setRange(0, items.size() - 1);

            // Если элементы закончились, очищаем поля
            if (items.isEmpty()) {
                textEdit->clear();
                titleEdit->clear();
                passwordEdit->clear();
                lastTimeEdit->clear();
                countEdit->clear();
            }
            else {
                // Переход к предыдущему элементу (если удален не первый)
                if (index > 0) {
                    slider->setValue(index - 1);
                }
                else {
                    slider->setValue(0);
                }
            }

            // Сохранение изменений в файл
            saveFile();

            QMessageBox::information(this, "Удалено", "Задача успешно удалена!");
        }
    }

    void saveFile() {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);

            // Формирование содержимого файла
            for (int i = 0; i < items.size(); ++i) {
                QString item = items[i];
                QString title = titles[i];
                QString password = passwords[i];

                // Добавляем название и пароль обратно в элемент
                if (!title.isEmpty()) {
                    item.prepend("$$" + title + "$$ ");
                }
                if (!password.isEmpty()) {
                    item.prepend("~~" + password + "~~ ");
                }

                out << item;
                if (i < items.size() - 1) {
                    out << SEP; // Добавляем разделитель между элементами
                }
            }

            file.close();
        }
        else {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл для записи!");
        }
    }

private:
    QTextEdit* textEdit;
    QLineEdit* titleEdit;
    QLineEdit* passwordEdit;
    QLineEdit* lastTimeEdit;
    QLineEdit* countEdit;
    QSlider* slider;
    QStringList items;
    QStringList titles;
    QStringList passwords;
    QString filePath; // Путь к файлу
};
class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    MainWindow(QWidget * parent = nullptr) : QMainWindow(parent) {
        // Создаем центральный виджет и устанавливаем его в MainWindow
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // Создаем вертикальный layout для центрального виджета
        QVBoxLayout* layout = new QVBoxLayout(centralWidget);

        // Создаем первую кнопку
        QPushButton* button1 = new QPushButton("Открыть окно редактирования списка задач.", this);
        layout->addWidget(button1);

        // Создаем вторую кнопку
        QPushButton* button2 = new QPushButton("Выполнить задачу.", this);
        layout->addWidget(button2);

        // Подключаем сигналы кнопок к слотам
        connect(button1, &QPushButton::clicked, this, &MainWindow::openTextFileViewer);
        connect(button2, &QPushButton::clicked, this, &MainWindow::openLineSubstringDialog);
    }

private slots:
    void openTextFileViewer() {
        // Укажите путь к вашему текстовому файлу
        QString filePath = "Textfile.txt";

        // Создаем и показываем окно TextFileViewer
        TextFileViewer* viewer = new TextFileViewer(filePath);
        viewer->setWindowTitle("Редактирование задач.");
        viewer->resize(400, 400); // Увеличиваем размер окна для нового поля
        viewer->show();
    }

    void openLineSubstringDialog() {
        // Создаем диалоговое окно
        QDialog dialog(this);
        dialog.setWindowTitle("Выполнить задачу");

        // Создаем layout для диалога
        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        // Поле для ввода подстроки
        QLineEdit* substringEdit = new QLineEdit(&dialog);
        substringEdit->setPlaceholderText("Введите название задачи.");
        layout->addWidget(substringEdit);

        // Поле для ввода пароля
        QLineEdit* passwordEdit = new QLineEdit(&dialog);
        passwordEdit->setPlaceholderText("Введите пароль.");
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
            QString substring = substringEdit->text();

            // Автоматически добавляем $$ к подстроке
            substring = "$$" + substring + "$$";

            // Укажите путь к вашему текстовому файлу
            QString filePath = "Textfile.txt";

            // Загружаем файл
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QStringList lines = in.readAll().split(SEP);
                file.close();

                // Ищем строку, содержащую подстроку
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
                        // Извлекаем подстроку, заключенную в $$
                        QRegularExpression substringRegex("\\$\\$(.*?)\\$\\$");
                        QRegularExpressionMatch substringMatch = substringRegex.match(selectedLine);

                        if (substringMatch.hasMatch()) {
                            QString extractedSubstring = substringMatch.captured(1); // Извлекаем подстроку без $$

                            // Создаем новое диалоговое окно для отображения строки
                            QDialog lineDialog(this);
                            lineDialog.setWindowTitle("Содержание задачи.");

                            QVBoxLayout* lineLayout = new QVBoxLayout(&lineDialog);

                            // Отображаем текст строки
                            QLabel* lineLabel = new QLabel(selectedLine, &lineDialog);
                            lineLayout->addWidget(lineLabel);

                            // Кнопка "Ready" для записи подстроки в файл
                            QPushButton* readyButton = new QPushButton("Выполнить.", &lineDialog);
                            lineLayout->addWidget(readyButton);

                            // Подключаем кнопку "Ready" к слоту
                            connect(readyButton, &QPushButton::clicked, [&lineDialog, extractedSubstring]() {
                                // Получаем текущее время
                                QDateTime currentTime = QDateTime::currentDateTime();
                                QString timeString = currentTime.toString("yyyy-MM-dd HH:mm:ss");

                                // Записываем подстроку и время в файл Results.txt
                                QFile resultsFile("Results.txt");
                                if (resultsFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                                    QTextStream out(&resultsFile);
                                    out << "Selected line: " << extractedSubstring << " | Time: " << timeString << "\n";
                                    resultsFile.close();
                                    QMessageBox::information(&lineDialog, "Успешно.", "Факт выполнеиния успешно сохранён!");
                                }
                                else {
                                    QMessageBox::critical(&lineDialog, "Ошибка", "Ошибка сохранения.");
                                }
                                lineDialog.close();
                                });

                            // Показываем диалог
                            lineDialog.exec();
                        }
                        else {
                            QMessageBox::critical(this, "Ошибка.", "Такая задача не найдена.");
                        }
                    }
                    else {
                        // Сообщение об ошибке
                        QMessageBox::critical(this, "Ошибка.", "Неверный пароль.");
                    }
                }
                else {
                    // Сообщение об ошибке
                    QMessageBox::critical(this, "Ошибка.", "Задача не найдена.");
                }
            }
            else {
                QMessageBox::critical(this, "Ошибка.", "Ошибка открытия файла.");
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