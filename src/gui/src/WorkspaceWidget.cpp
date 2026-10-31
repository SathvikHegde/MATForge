#include "matforge/gui/WorkspaceWidget.hpp"

#include <QVBoxLayout>
#include <QHeaderView>

namespace matforge::gui {

WorkspaceWidget::WorkspaceWidget(std::shared_ptr<matforge::engine::Workspace> workspace, QWidget* parent)
    : QWidget(parent), workspace_(std::move(workspace)) {
    setupUi();
    refresh();

    if (workspace_) {
        workspace_->setChangeCallback([this](const std::string&, const matforge::core::Value&) {
            QMetaObject::invokeMethod(this, [this]() { refresh(); }, Qt::QueuedConnection);
        });
        workspace_->setClearCallback([this]() {
            QMetaObject::invokeMethod(this, [this]() { refresh(); }, Qt::QueuedConnection);
        });
    }
}

void WorkspaceWidget::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    table_ = new QTableWidget(this);
    table_->setColumnCount(5);
    table_->setHorizontalHeaderLabels({"Name", "Value", "Size", "Bytes", "Class"});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->setVisible(false);
    table_->setAlternatingRowColors(true);

    layout->addWidget(table_);

    connect(table_, &QTableWidget::itemDoubleClicked, this, &WorkspaceWidget::onItemDoubleClicked);
}

void WorkspaceWidget::refresh() {
    if (!workspace_ || !table_) return;

    table_->setRowCount(0);
    int row = 0;

    for (const auto& [name, val] : workspace_->variables()) {
        if (name == "pi" || name == "e" || name == "eps" || name == "Inf" || name == "NaN") continue;

        table_->insertRow(row);

        auto* nameItem = new QTableWidgetItem(QString::fromStdString(name));
        auto* valItem = new QTableWidgetItem(QString::fromStdString(val.previewString()));
        auto* sizeItem = new QTableWidgetItem(QString::fromStdString(val.sizeString()));
        auto* bytesItem = new QTableWidgetItem(QString::number(val.byteSize()));
        auto* classItem = new QTableWidgetItem(QString::fromStdString(val.typeName()));

        table_->setItem(row, 0, nameItem);
        table_->setItem(row, 1, valItem);
        table_->setItem(row, 2, sizeItem);
        table_->setItem(row, 3, bytesItem);
        table_->setItem(row, 4, classItem);

        row++;
    }
}

void WorkspaceWidget::onItemDoubleClicked(QTableWidgetItem* item) {
    if (!item) return;
    int row = item->row();
    QTableWidgetItem* nameItem = table_->item(row, 0);
    if (nameItem) {
        emit variableInspected(nameItem->text());
    }
}

} // namespace matforge::gui

