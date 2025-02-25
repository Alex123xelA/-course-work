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


// ����� TextFileViewer � ������������ ��������������
class TextFileViewer : public QWidget {
    Q_OBJECT

public:
    TextFileViewer(const QString& filePath, QWidget* parent = nullptr)
        : QWidget(parent), filePath(filePath) {
        loadFile();

        QVBoxLayout* layout = new QVBoxLayout(this);

        // ��������� ���� ��� ����������� � �������������� ������
        textEdit = new QTextEdit(this);
        textEdit->setReadOnly(false); // ��������� ��������������
        layout->addWidget(textEdit);

        // �������� ��� ������ ������
        slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, lines.size() - 1);
        slider->setValue(0);
        layout->addWidget(slider);

        // ������ ��� ���������� ���������
        QPushButton* saveButton = new QPushButton("Save Changes", this);
        layout->addWidget(saveButton);

        // ���������� �������
        connect(slider, &QSlider::valueChanged, this, &TextFileViewer::updateText);
        connect(saveButton, &QPushButton::clicked, this, &TextFileViewer::saveChanges);

        // ������������� ������
        updateText(0);
    }

private slots:
    // ���������� ������ ��� ��������� ��������
    void updateText(int value) {
        if (value >= 0 && value < lines.size()) {
            textEdit->setText(lines[value]);
        }
    }

    // ���������� ��������� � ����
    void saveChanges() {
        int currentLine = slider->value();
        if (currentLine >= 0 && currentLine < lines.size()) {
            // ��������� ������ � ������
            lines[currentLine] = textEdit->toPlainText();

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
    QSlider* slider;
};

// ����� MainWindow
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
        QPushButton* button1 = new QPushButton("Open TextFileViewer", this);
        layout->addWidget(button1);

        // ������� ������ ������
        QPushButton* button2 = new QPushButton("Select Line by Number", this);
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
        viewer->setWindowTitle("Text File Viewer");
        viewer->resize(400, 300);
        viewer->show();
    }

    void openLineNumberDialog() {
        // ������� ���������� ����
        QDialog dialog(this);
        dialog.setWindowTitle("Select Line Number");

        // ������� layout ��� �������
        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        // ���� ��� ����� ������ ������
        QLineEdit* lineEdit = new QLineEdit(&dialog);
        lineEdit->setPlaceholderText("Enter line number");
        layout->addWidget(lineEdit);

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
                        // ���������� ����� ������
                        QMessageBox::information(this, "Line Text", lines[lineNumber - 1]);
                    }
                    else {
                        // ��������� �� ������
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

    // ������� � ���������� MainWindow
    MainWindow mainWindow;
    mainWindow.setWindowTitle("Main Window");
    mainWindow.resize(300, 200);
    mainWindow.show();

    return app.exec();
}

#include "main.moc"