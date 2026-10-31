#include "matforge/gui/CommandHistoryWidget.hpp"

#include <QVBoxLayout>
#include <QDateTime>

namespace matforge::gui {

CommandHistoryWidget::CommandHistoryWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    listWidget_ = new QListWidget(this);
    listWidget_->setAlternatingRowColors(true);
    layout->addWidget(listWidget_);

    connect(listWidget_, &QListWidget::itemDoubleClicked, this, &CommandHistoryWidget::onItemDoubleClicked);
}

void CommandHistoryWidget::addCommand(const QString& command) {
    if (command.trimmed().isEmpty()) return;
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString entry = QString("[%1] %2").arg(timeStr, command);
    auto* item = new QListWidgetItem(entry, listWidget_);
    item->setData(Qt::UserRole, command);
    listWidget_->scrollToBottom();
}

void CommandHistoryWidget::clearHistory() {
    listWidget_->clear();
}

void CommandHistoryWidget::onItemDoubleClicked(QListWidgetItem* item) {
    if (item) {
        emit commandSelected(item->data(Qt::UserRole).toString());
    }
}

} // namespace matforge::gui

