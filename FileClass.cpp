#include "TaskApp.h"

DataFile::DataFile(const QString& filePath, QObject* parent)
    : QFile(filePath, parent)
{
}

bool DataFile::loadFile(QString& content)
{
    if (!open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(this);
    content = in.readAll();
    close();

    return true;
}

QList<QStringList> DataFile::splitContentToPairs(const QString& content,
    const QString& SEP,
    const QString& SEPNAME) const
{
    QList<QStringList> result;

    if (content.isEmpty()) {
        return result;
    }

    // Сначала разбиваем по основному разделителю SEP
    QStringList lines = content.split(SEP, Qt::SkipEmptyParts);

    foreach(const QString & line, lines) {
        // Затем каждую строку разбиваем по SEPNAME
        QStringList parts = line.split(SEPNAME, Qt::SkipEmptyParts);

        if (parts.size() >= 2) {
            // Если есть оба компонента
            QStringList pair;
            pair.append(parts[0].trimmed());    // Первая часть (имя)
            pair.append(parts[1].trimmed());    // Вторая часть (значение)
            result.append(pair);
        }
        else if (!parts.isEmpty()) {
            // Если только один компонент
            QStringList pair;
            pair.append(parts[0].trimmed());    // Первая часть (имя)
            pair.append("");                   // Пустое значение
            result.append(pair);
        }
    }

    return result;
}