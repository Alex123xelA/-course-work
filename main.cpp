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

        // ��������� ���� ��� ����������� � �������������� ������ (��� ����� � ~~)
        textEdit = new QTextEdit(this);
        textEdit->setReadOnly(false); // ��������� ��������������
        layout->addWidget(textEdit);

        // ����� ��������� ���� ��� �����, ����������� � ~~
        specialTextEdit = new QTextEdit(this);
        specialTextEdit->setReadOnly(false); // ��������� ��������������
        specialTextEdit->setPlaceholderText("Edit password lines (~~text~~) here");
        layout->addWidget(specialTextEdit);

        // �������� ��� ������ ������
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, lines.size() - 1);
        slider->setValue(0);
        layout->addWidget(slider);

        // ������ ��� ���������� ���������
        QPushButton* saveButton = new QPushButton("Save Changes", this);
        layout->addWidget(saveButton);

        // ����� ������ ��� ���������� ������ �����������
        QPushButton* addSeparatorButton = new QPushButton("Add New Task", this);
        layout->addWidget(addSeparatorButton);

        // ������ ��� �������� ������� ������
        QPushButton* deleteButton = new QPushButton("Delete Current Task", this);
        layout->addWidget(deleteButton);

        // ���������� �������
        connect(slider, &QSlider::valueChanged, this, &TextFileViewer::updateText);
        connect(saveButton, &QPushButton::clicked, this, &TextFileViewer::saveChanges);
        connect(addSeparatorButton, &QPushButton::clicked, this, &TextFileViewer::addNewLine);
        connect(deleteButton, &QPushButton::clicked, this, &TextFileViewer::deleteCurrentLine);

        // ������������� ������
        updateText(0);
    }

private slots:
    // ���������� ������ ��� ��������� ��������
    void updateText(int value) {
        if (value >= 0 && value < lines.size()) {
            QString currentLine = lines[value];

            // ������� ������, ����������� � ~~, ��� ����������� � textEdit
            QRegularExpression regex("~~.*?~~");
            QString displayText = currentLine;
            displayText.remove(regex); // ������� ��� ��������� ~~������~~
            textEdit->setText(displayText);

            // ���� ������, ����������� � ~~, � ���������� �� � specialTextEdit
            QRegularExpression specialRegex("~~(.*?)~~");
            QRegularExpressionMatchIterator matches = specialRegex.globalMatch(currentLine);

            QStringList specialLines;
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                specialLines.append(match.captured(1)); // ��������� ����� ������ ~~
            }

            // ���������� ��������� ������ � specialTextEdit
            specialTextEdit->setText(specialLines.join("\n"));
        }
    }

    // ���������� ��������� � ����
    void saveChanges() {
        int currentLine = slider->value();
        if (currentLine >= 0 && currentLine < lines.size()) {
            // ��������� ������ � ������
            QString updatedLine = textEdit->toPlainText();

            // ��������� ������, ����������� � ~~, �� specialTextEdit
            QStringList specialLines = specialTextEdit->toPlainText().split("\n");
            QRegularExpression regex("~~(.*?)~~");
            QRegularExpressionMatchIterator matches = regex.globalMatch(lines[currentLine]);

            int specialIndex = 0;
            QString newLine = lines[currentLine];
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                if (specialIndex < specialLines.size()) {
                    // �������� ����� ������ ~~ �� �����
                    newLine.replace(match.captured(0), "~~" + specialLines[specialIndex] + "~~");
                    specialIndex++;
                }
            }

            // ��������� ������
            lines[currentLine] = newLine;

            // ��������� ��� ������ � ����
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

    // ���������� ������ ����������� � ������ ������
    void addNewLine() {
        // ���������, ���� �� ������, ����������� � ~~
        bool hasSpecialLine = false;
        for (const QString& line : lines) {
            if (line.contains("~~")) {
                hasSpecialLine = true;
                break;
            }
        }

        if (!hasSpecialLine) {
            // ���� ��� ������ � ~~, ���������� ������
            QMessageBox::critical(this, "Error", "No task with password found! Cannot add a new task.");
            return;
        }

        // ��������� ����� ����������� � ������ ������
        lines.append("");
        slider->setRange(0, lines.size() - 1);
        slider->setValue(lines.size() - 1);
        updateText(lines.size() - 1);
    }

    // �������� ������� ������
    void deleteCurrentLine() {
        int currentLine = slider->value();
        if (currentLine >= 0 && currentLine < lines.size()) {
            // ������� ������� ������
            lines.removeAt(currentLine);

            // ��������� ��������
            slider->setRange(0, lines.size() - 1);

            // ���� ����� ������ ���, ������� ��������� ����
            if (lines.isEmpty()) {
                textEdit->clear();
                specialTextEdit->clear();
            }
            else {
                // ������������� �� ���������� ������
                if (currentLine >= lines.size()) {
                    currentLine = lines.size() - 1;
                }
                slider->setValue(currentLine);
                updateText(currentLine);
            }

            // ��������� ��������� � ����
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
    // �������� ����� �� �����
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
    QTextEdit* specialTextEdit; // ����� ���� ��� �����, ����������� � ~~
    QSlider* slider;
};

// ����� �������� ����
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        // ������� ����������� ������ � ������������� ��� � MainWindow
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // ������� ������������ layout ��� ������������ �������
        QVBoxLayout* layout = new QVBoxLayout(centralWidget);

        // ������� ������ ������
        QPushButton* button1 = new QPushButton("Open Create Tasks Window", this);
        layout->addWidget(button1);

        // ������� ������ ������
        QPushButton* button2 = new QPushButton("Select Task by Number", this);
        layout->addWidget(button2);

        // ���������� ������� ������ � ������
        connect(button1, &QPushButton::clicked, this, &MainWindow::openTextFileViewer);
        connect(button2, &QPushButton::clicked, this, &MainWindow::openLineNumberDialog);
    }

private slots:
    void openTextFileViewer() {
        // ������� ���� � ������ ���������� �����
        QString filePath = "Textfile.txt";

        // ������� � ���������� ���� TextFileViewer
        TextFileViewer* viewer = new TextFileViewer(filePath);
        viewer->setWindowTitle("Create Tasks");
        viewer->resize(400, 400); // ����������� ������ ���� ��� ������ ����
        viewer->show();
    }

    void openLineNumberDialog() {
        // ������� ���������� ����
        QDialog dialog(this);
        dialog.setWindowTitle("Select Task Number");

        // ������� layout ��� �������
        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        // ���� ��� ����� ������ ������
        QLineEdit* lineEdit = new QLineEdit(&dialog);
        lineEdit->setPlaceholderText("Enter task number");
        layout->addWidget(lineEdit);

        // ���� ��� ����� ������
        QLineEdit* passwordEdit = new QLineEdit(&dialog);
        passwordEdit->setPlaceholderText("Enter password");
        passwordEdit->setEchoMode(QLineEdit::Password); // �������� ���� ������
        layout->addWidget(passwordEdit);

        // ������ ������������� � ������
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttonBox);

        // ���������� ������ � ������
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        // ���������� ������ � ������������ ���������
        if (dialog.exec() == QDialog::Accepted) {
            bool ok;
            int lineNumber = lineEdit->text().toInt(&ok);

            if (ok) {
                // ������� ���� � ������ ���������� �����
                QString filePath = "Textfile.txt";

                // ��������� ����
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    QStringList lines = in.readAll().split(SEP);
                    file.close();

                    // ���������, ���������� �� ������ � ����� �������
                    if (lineNumber >= 1 && lineNumber <= lines.size()) {
                        QString selectedLine = lines[lineNumber - 1];

                        // ���������, ��������� �� ��������� ������ � ����� ������� � ~~
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
                            // ���������� ����� ������
                            QMessageBox::information(this, "Task Text", selectedLine);
                        }
                        else {
                            // ��������� �� ������
                            QMessageBox::critical(this, "Error", "Incorrect password!");
                        }
                    }
                    else {
                        // ��������� �� ������
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

    // ������� � ���������� MainWindow
    MainWindow mainWindow;
    mainWindow.setWindowTitle("Main Window");
    mainWindow.resize(300, 200);
    mainWindow.show();

    return app.exec();
}

#include "main.moc"