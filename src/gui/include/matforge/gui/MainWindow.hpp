#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <memory>

namespace matforge::engine {
class Interpreter;
class Workspace;
}

namespace matforge::gui {

class CommandWindow;
class CodeEditor;
class WorkspaceWidget;
class FileBrowserWidget;
class CommandHistoryWidget;
class FigureWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNewScript();
    void onOpenFile();
    void onSaveFile();
    void onRunScript();
    void onCommandSubmitted(const QString& command);
    void onFileDoubleClicked(const QString& filePath);
    void onVariableInspected(const QString& varName);
    void onPlotRequested(const std::vector<double>& x, const std::vector<double>& y, const std::string& title);

private:
    void setupUi();
    void setupDocks();
    void setupToolbars();
    void setupMenus();

    std::shared_ptr<matforge::engine::Workspace> workspace_;
    std::unique_ptr<matforge::engine::Interpreter> interpreter_;

    QTabWidget* editorTabs_{nullptr};
    CommandWindow* commandWindow_{nullptr};
    WorkspaceWidget* workspaceWidget_{nullptr};
    FileBrowserWidget* fileBrowserWidget_{nullptr};
    CommandHistoryWidget* historyWidget_{nullptr};
    FigureWindow* figureWindow_{nullptr};
};

} // namespace matforge::gui

