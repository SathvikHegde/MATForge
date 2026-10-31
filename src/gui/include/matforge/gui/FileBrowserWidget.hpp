#pragma once

#include <QWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QLineEdit>
#include <QToolButton>

namespace matforge::gui {

class FileBrowserWidget : public QWidget {
    Q_OBJECT

public:
    explicit FileBrowserWidget(const QString& initialPath = "", QWidget* parent = nullptr);

    void setRootPath(const QString& path);
    [[nodiscard]] QString currentPath() const;

signals:
    void fileDoubleClicked(const QString& filePath);

private slots:
    void onUpDirectoryClicked();
    void onTreeItemDoubleClicked(const QModelIndex& index);

private:
    void setupUi();

    QLineEdit* pathLineEdit_{nullptr};
    QToolButton* upButton_{nullptr};
    QTreeView* treeView_{nullptr};
    QFileSystemModel* model_{nullptr};
};

} // namespace matforge::gui

