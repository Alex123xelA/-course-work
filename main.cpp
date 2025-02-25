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


const QString SEP = "`~`&";


// Класс TextFileViewer с возможностью редактирования
class TextFileViewer : public QWidget {
    Q_OBJECT

public:
    TextFileViewer(const QString& filePath, QWidget* parent = nullptr)
        : QWidget(parent), filePath(filePath) {
        loadFile();

        QVBoxLayout* layout = new QVBoxLayout(this);

        // Текстовое поле для отображения и редактирования строки
        textEdit = new QTextEdit(this);
        textEdit->setReadOnly(false); // Разрешаем редактирование
        layout->addWidget(textEdit);

        // Ползунок для выбора строки
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, lines.size() - 1);
        slider->setValue(0);
        layout->addWidget(slider);

        // Кнопка для сохранения изменений
        QPushButton* saveButton = new QPushButton("Save Changes", this);
        layout->addWidget(saveButton);

        // Подключаем сигналы
        connect(slider, &QSlider::valueChanged, this, &TextFileViewer::updateText);
        connect(saveButton, &QPushButton::clicked, this, &TextFileViewer::saveChanges);

        // Инициализация текста
        updateText(0);
    }

private slots:
    // Обновление текста при изменении ползунка
    void updateText(int value) {
        if (value >= 0 && value < lines.size()) {
            textEdit->setText(lines[value]);
        }
    }

    // Сохранение изменений в файл
    void saveChanges() {
        int currentLine = slider->value();
        if (currentLine >= 0 && currentLine < lines.size()) {
            // Обновляем строку в списке
            lines[currentLine] = textEdit->toPlainText();

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
    QSlider* slider;
};

// Класс MainWindow
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
        QPushButton* button1 = new QPushButton("Open TextFileViewer", this);
        layout->addWidget(button1);

        // Создаем вторую кнопку
        QPushButton* button2 = new QPushButton("Select Line by Number", this);
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
        viewer->setWindowTitle("Text File Viewer");
        viewer->resize(400, 300);
        viewer->show();
    }

    void openLineNumberDialog() {
        // Создаем диалоговое окно
        QDialog dialog(this);
        dialog.setWindowTitle("Select Line Number");

        // Создаем layout для диалога
        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        // Поле для ввода номера строки
        QLineEdit* lineEdit = new QLineEdit(&dialog);
        lineEdit->setPlaceholderText("Enter line number");
        layout->addWidget(lineEdit);

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
                        // Отображаем текст строки
                        QMessageBox::information(this, "Line Text", lines[lineNumber - 1]);
                    }
                    else {
                        // Сообщение об ошибке
                        QMessageBox::critical(this, "Error", "Invalid line number!");
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