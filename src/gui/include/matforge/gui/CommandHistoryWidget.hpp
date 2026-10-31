#pragma once

#include <QWidget>
#include <QListWidget>

namespace matforge::gui {

class CommandHistoryWidget : public QWidget {
    Q_OBJECT

public:
    explicit CommandHistoryWidget(QWidget* parent = nullptr);

    void addCommand(const QString& command);
    void clearHistory();

signals:
    void commandSelected(const QString& command);

private slots:
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    QListWidget* listWidget_{nullptr};
};

} // namespace matforge::gui

