#include "matforge/gui/VariableEditorDialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>

namespace matforge::gui {

VariableEditorDialog::VariableEditorDialog(const QString& varName,
                                           std::shared_ptr<matforge::engine::Workspace> workspace,
                                           QWidget* parent)
    : QDialog(parent), varName_(varName), workspace_(std::move(workspace)) {
    setWindowTitle("Variable: " + varName_);
    resize(600, 400);

    setupUi();
    loadData();
}

void VariableEditorDialog::setupUi() {
    auto* layout = new QVBoxLayout(this);

    // Top info bar
    infoLabel_ = new QLabel(this);
    infoLabel_->setStyleSheet("font-weight: bold; padding: 4px; color: #333;");
    layout->addWidget(infoLabel_);

    // Data table
    table_ = new QTableWidget(this);
    table_->setAlternatingRowColors(true);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    layout->addWidget(table_);

    // Action buttons
    auto* btnLayout = new QHBoxLayout();
    auto* addRowBtn = new QPushButton("Add Row", this);
    auto* addColBtn = new QPushButton("Add Column", this);
    auto* exportBtn = new QPushButton("Export CSV...", this);
    auto* closeBtn = new QPushButton("Close", this);

    btnLayout->addWidget(addRowBtn);
    btnLayout->addWidget(addColBtn);
    btnLayout->addWidget(exportBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(table_, &QTableWidget::cellChanged, this, &VariableEditorDialog::onCellChanged);
    connect(addRowBtn, &QPushButton::clicked, this, &VariableEditorDialog::onAddRow);
    connect(addColBtn, &QPushButton::clicked, this, &VariableEditorDialog::onAddColumn);
    connect(exportBtn, &QPushButton::clicked, this, &VariableEditorDialog::onExportCsv);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void VariableEditorDialog::loadData() {
    if (!workspace_ || !workspace_->has(varName_.toStdString())) return;

    updating_ = true;
    auto val = workspace_->get(varName_.toStdString());

    infoLabel_->setText(QString("Name: %1 | Type: %2 | Size: %3 | Bytes: %4")
                        .arg(varName_)
                        .arg(QString::fromStdString(val.typeName()))
                        .arg(QString::fromStdString(val.sizeString()))
                        .arg(val.byteSize()));

    if (val.isNumeric()) {
        auto mat = val.toMatrix();
        table_->setRowCount(static_cast<int>(mat.rows()));
        table_->setColumnCount(static_cast<int>(mat.cols()));

        QStringList colHeaders;
        for (std::size_t c = 0; c < mat.cols(); ++c) {
            colHeaders << QString::number(c + 1);
        }
        table_->setHorizontalHeaderLabels(colHeaders);

        QStringList rowHeaders;
        for (std::size_t r = 0; r < mat.rows(); ++r) {
            rowHeaders << QString::number(r + 1);
        }
        table_->setVerticalHeaderLabels(rowHeaders);

        for (std::size_t r = 0; r < mat.rows(); ++r) {
            for (std::size_t c = 0; c < mat.cols(); ++c) {
                auto* item = new QTableWidgetItem(QString::number(mat(r, c), 'g', 6));
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                table_->setItem(static_cast<int>(r), static_cast<int>(c), item);
            }
        }
    } else if (val.isString()) {
        table_->setRowCount(1);
        table_->setColumnCount(1);
        table_->setHorizontalHeaderLabels({"1"});
        table_->setVerticalHeaderLabels({"1"});
        auto* item = new QTableWidgetItem(QString::fromStdString(val.toStringValue()));
        table_->setItem(0, 0, item);
    }

    updating_ = false;
}

void VariableEditorDialog::onCellChanged(int row, int col) {
    if (updating_ || !workspace_ || !workspace_->has(varName_.toStdString())) return;

    auto val = workspace_->get(varName_.toStdString());
    if (val.isNumeric()) {
        auto mat = val.toMatrix();
        if (static_cast<std::size_t>(row) < mat.rows() && static_cast<std::size_t>(col) < mat.cols()) {
            QTableWidgetItem* item = table_->item(row, col);
            if (item) {
                bool ok = false;
                double d = item->text().toDouble(&ok);
                if (ok) {
                    mat(static_cast<std::size_t>(row), static_cast<std::size_t>(col)) = d;
                    workspace_->set(varName_.toStdString(), matforge::core::Value(mat));
                }
            }
        }
    }
}

void VariableEditorDialog::onAddRow() {
    if (!workspace_ || !workspace_->has(varName_.toStdString())) return;
    auto val = workspace_->get(varName_.toStdString());
    if (!val.isNumeric()) return;

    auto mat = val.toMatrix();
    std::size_t newRows = mat.rows() + 1;
    std::size_t cols = mat.cols() == 0 ? 1 : mat.cols();

    matforge::core::Matrix newMat(newRows, cols, 0.0);
    for (std::size_t r = 0; r < mat.rows(); ++r) {
        for (std::size_t c = 0; c < mat.cols(); ++c) {
            newMat(r, c) = mat(r, c);
        }
    }
    workspace_->set(varName_.toStdString(), matforge::core::Value(newMat));
    loadData();
}

void VariableEditorDialog::onAddColumn() {
    if (!workspace_ || !workspace_->has(varName_.toStdString())) return;
    auto val = workspace_->get(varName_.toStdString());
    if (!val.isNumeric()) return;

    auto mat = val.toMatrix();
    std::size_t rows = mat.rows() == 0 ? 1 : mat.rows();
    std::size_t newCols = mat.cols() + 1;

    matforge::core::Matrix newMat(rows, newCols, 0.0);
    for (std::size_t r = 0; r < mat.rows(); ++r) {
        for (std::size_t c = 0; c < mat.cols(); ++c) {
            newMat(r, c) = mat(r, c);
        }
    }
    workspace_->set(varName_.toStdString(), matforge::core::Value(newMat));
    loadData();
}

void VariableEditorDialog::onExportCsv() {
    if (!workspace_ || !workspace_->has(varName_.toStdString())) return;
    auto val = workspace_->get(varName_.toStdString());
    if (!val.isNumeric()) {
        QMessageBox::warning(this, "Export", "Only numeric matrices can be exported to CSV.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Export Matrix to CSV", varName_ + ".csv", "CSV Files (*.csv);;All Files (*.*)");
    if (fileName.isEmpty()) return;

    try {
        val.toMatrix().writeCSV(fileName.toStdString());
        QMessageBox::information(this, "Export Successful", "Matrix successfully exported to " + fileName);
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Export Error", QString("Failed to export: %1").arg(ex.what()));
    }
}

} // namespace matforge::gui

