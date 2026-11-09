#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <memory>
#include "matforge/engine/Workspace.hpp"

namespace matforge::gui {

class VariableEditorDialog : public QDialog {
    Q_OBJECT

public:
    VariableEditorDialog(const QString& varName, 
                         std::shared_ptr<matforge::engine::Workspace> workspace, 
                         QWidget* parent = nullptr);

private slots:
    void onCellChanged(int row, int col);
    void onAddRow();
    void onAddColumn();
    void onExportCsv();

private:
    void setupUi();
    void loadData();

    QString varName_;
    std::shared_ptr<matforge::engine::Workspace> workspace_;

    QLabel* infoLabel_{nullptr};
    QTableWidget* table_{nullptr};
    bool updating_{false};
};

} // namespace matforge::gui

