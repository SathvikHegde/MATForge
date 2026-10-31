#pragma once

#include <QWidget>
#include <QTableWidget>
#include <memory>
#include "matforge/engine/Workspace.hpp"

namespace matforge::gui {

class WorkspaceWidget : public QWidget {
    Q_OBJECT

public:
    explicit WorkspaceWidget(std::shared_ptr<matforge::engine::Workspace> workspace, QWidget* parent = nullptr);

    void refresh();

signals:
    void variableInspected(const QString& varName);

private slots:
    void onItemDoubleClicked(QTableWidgetItem* item);

private:
    void setupUi();

    std::shared_ptr<matforge::engine::Workspace> workspace_;
    QTableWidget* table_{nullptr};
};

} // namespace matforge::gui

