#include "matforge/gui/FileBrowserWidget.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QHeaderView>

namespace matforge::gui {

FileBrowserWidget::FileBrowserWidget(const QString& initialPath, QWidget* parent)
    : QWidget(parent) {
    setupUi();
    QString startPath = initialPath.isEmpty() ? QDir::currentPath() : initialPath;
    setRootPath(startPath);
}

void FileBrowserWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Path bar
    auto* topLayout = new QHBoxLayout();
    pathLineEdit_ = new QLineEdit(this);
    pathLineEdit_->setReadOnly(true);

    upButton_ = new QToolButton(this);
    upButton_->setText("..");
    upButton_->setToolTip("Up to parent folder");
    connect(upButton_, &QToolButton::clicked, this, &FileBrowserWidget::onUpDirectoryClicked);

    topLayout->addWidget(pathLineEdit_);
    topLayout->addWidget(upButton_);
    mainLayout->addLayout(topLayout);

    // Tree View
    model_ = new QFileSystemModel(this);
    model_->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    treeView_ = new QTreeView(this);
    treeView_->setModel(model_);
    treeView_->hideColumn(2); // Hide file type
    treeView_->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(treeView_, &QTreeView::doubleClicked, this, &FileBrowserWidget::onTreeItemDoubleClicked);

    mainLayout->addWidget(treeView_);
}

void FileBrowserWidget::setRootPath(const QString& path) {
    QModelIndex rootIndex = model_->setRootPath(path);
    treeView_->setRootIndex(rootIndex);
    pathLineEdit_->setText(path);
}

QString FileBrowserWidget::currentPath() const {
    return pathLineEdit_->text();
}

void FileBrowserWidget::onUpDirectoryClicked() {
    QDir dir(currentPath());
    if (dir.cdUp()) {
        setRootPath(dir.absolutePath());
    }
}

void FileBrowserWidget::onTreeItemDoubleClicked(const QModelIndex& index) {
    QString path = model_->filePath(index);
    if (model_->isDir(index)) {
        setRootPath(path);
    } else {
        emit fileDoubleClicked(path);
    }
}

} // namespace matforge::gui

