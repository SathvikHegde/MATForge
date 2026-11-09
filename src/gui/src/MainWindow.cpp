#include "matforge/gui/MainWindow.hpp"
#include "matforge/gui/CommandWindow.hpp"
#include "matforge/gui/CodeEditor.hpp"
#include "matforge/gui/WorkspaceWidget.hpp"
#include "matforge/gui/FileBrowserWidget.hpp"
#include "matforge/gui/CommandHistoryWidget.hpp"
#include "matforge/gui/FigureWindow.hpp"
#include "matforge/engine/Interpreter.hpp"
#include "matforge/engine/Workspace.hpp"
#include "matforge/engine/Lexer.hpp"

#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

namespace matforge::gui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      workspace_(std::make_shared<matforge::engine::Workspace>()),
      interpreter_(std::make_unique<matforge::engine::Interpreter>(workspace_)),
      figureWindow_(new FigureWindow(this)) {
    setupUi();

    interpreter_->setPlotCallback([this](const std::vector<double>& x, const std::vector<double>& y, const std::string& title) {
        onPlotRequested(x, y, title);
    });

    interpreter_->setClearScreenCallback([this]() {
        if (commandWindow_) {
            commandWindow_->clearWindow();
        }
    });

    interpreter_->setCloseFigureCallback([this]() {
        QMetaObject::invokeMethod(this, [this]() {
            if (figureWindow_) {
                figureWindow_->close();
            }
        }, Qt::QueuedConnection);
    });

    interpreter_->setClearFigureCallback([this]() {
        QMetaObject::invokeMethod(this, [this]() {
            if (figureWindow_) {
                figureWindow_->clearPlot();
            }
        }, Qt::QueuedConnection);
    });

    onNewScript();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("MATForge - MATLAB Clone IDE");
    resize(1280, 800);

    // Central Tabbed Code Editor
    editorTabs_ = new QTabWidget(this);
    editorTabs_->setTabsClosable(true);
    connect(editorTabs_, &QTabWidget::tabCloseRequested, [this](int index) {
        if (editorTabs_->count() > 1) {
            editorTabs_->removeTab(index);
        }
    });
    setCentralWidget(editorTabs_);

    setupDocks();
    setupToolbars();
    setupMenus();

    statusBar()->showMessage("Ready", 3000);
}

void MainWindow::setupDocks() {
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks | QMainWindow::AllowNestedDocks);

    // 1. Current Folder Dock (Left)
    auto* folderDock = new QDockWidget("Current Folder", this);
    fileBrowserWidget_ = new FileBrowserWidget("", folderDock);
    folderDock->setWidget(fileBrowserWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, folderDock);
    connect(fileBrowserWidget_, &FileBrowserWidget::fileDoubleClicked, this, &MainWindow::onFileDoubleClicked);

    // 2. Command Window Dock (Bottom)
    auto* cmdDock = new QDockWidget("Command Window", this);
    commandWindow_ = new CommandWindow(cmdDock);
    cmdDock->setWidget(commandWindow_);
    addDockWidget(Qt::BottomDockWidgetArea, cmdDock);
    connect(commandWindow_, &CommandWindow::commandSubmitted, this, &MainWindow::onCommandSubmitted);

    // 3. Workspace Dock (Right Top)
    auto* wsDock = new QDockWidget("Workspace", this);
    workspaceWidget_ = new WorkspaceWidget(workspace_, wsDock);
    wsDock->setWidget(workspaceWidget_);
    addDockWidget(Qt::RightDockWidgetArea, wsDock);
    connect(workspaceWidget_, &WorkspaceWidget::variableInspected, this, &MainWindow::onVariableInspected);

    // 4. Command History Dock (Right Bottom)
    auto* historyDock = new QDockWidget("Command History", this);
    historyWidget_ = new CommandHistoryWidget(historyDock);
    historyDock->setWidget(historyWidget_);
    addDockWidget(Qt::RightDockWidgetArea, historyDock);
    connect(historyWidget_, &CommandHistoryWidget::commandSelected, this, &MainWindow::onCommandSubmitted);
}

void MainWindow::setupToolbars() {
    auto* mainToolbar = addToolBar("Main Operations");
    mainToolbar->setMovable(false);

    QAction* actNew = mainToolbar->addAction("New Script");
    actNew->setShortcut(QKeySequence::New);
    connect(actNew, &QAction::triggered, this, &MainWindow::onNewScript);

    QAction* actOpen = mainToolbar->addAction("Open");
    actOpen->setShortcut(QKeySequence::Open);
    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpenFile);

    QAction* actSave = mainToolbar->addAction("Save");
    actSave->setShortcut(QKeySequence::Save);
    connect(actSave, &QAction::triggered, this, &MainWindow::onSaveFile);

    mainToolbar->addSeparator();

    QAction* actRun = mainToolbar->addAction("Run (F5)");
    actRun->setShortcut(QKeySequence(Qt::Key_F5));
    connect(actRun, &QAction::triggered, this, &MainWindow::onRunScript);

    mainToolbar->addSeparator();

    QAction* actClearWs = mainToolbar->addAction("Clear Workspace");
    connect(actClearWs, &QAction::triggered, [this]() {
        onCommandSubmitted("clear");
    });

    QAction* actClc = mainToolbar->addAction("Clear Console");
    connect(actClc, &QAction::triggered, [this]() {
        if (commandWindow_) commandWindow_->clearWindow();
    });

    mainToolbar->addSeparator();

    QAction* actFig = mainToolbar->addAction("Figure Window");
    connect(actFig, &QAction::triggered, [this]() {
        figureWindow_->show();
        figureWindow_->raise();
    });
}

void MainWindow::setupMenus() {
    auto* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("New Script", QKeySequence::New, this, &MainWindow::onNewScript);
    fileMenu->addAction("Open File...", QKeySequence::Open, this, &MainWindow::onOpenFile);
    fileMenu->addAction("Save File", QKeySequence::Save, this, &MainWindow::onSaveFile);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", QKeySequence::Quit, this, &QWidget::close);

    auto* editMenu = menuBar()->addMenu("&Editor");
    editMenu->addAction("Run Script", QKeySequence(Qt::Key_F5), this, &MainWindow::onRunScript);

    auto* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("About MATForge", [this]() {
        QMessageBox::about(this, "About MATForge",
                           "<h3>MATForge v0.1.0</h3>"
                           "<p>High-performance MATLAB clone created with C++20, Qt 6, and MSVC.</p>");
    });
}

void MainWindow::onNewScript() {
    auto* editor = new CodeEditor(editorTabs_);
    editor->setPlainText("% MATForge Sample Script (Phase 1 Features)\n"
                         "% 1. Matrix Creation & Slicing\n"
                         "M = [1, 2, 3; 4, 5, 6; 7, 8, 9];\n"
                         "corner = M(2:end, end-1:end);\n"
                         "\n"
                         "% 2. Control Flow & Summation\n"
                         "total = 0;\n"
                         "for i = 1:5\n"
                         "    total = total + i;\n"
                         "end\n"
                         "\n"
                         "% 3. Colon Ranges & Sine Wave Plot\n"
                         "t = 0:0.1:6.28;\n"
                         "y = sin(t);\n"
                         "plot(t, y);\n");
    int idx = editorTabs_->addTab(editor, QString("Untitled%1.m").arg(editorTabs_->count() + 1));
    editorTabs_->setCurrentIndex(idx);
}

void MainWindow::onOpenFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open MATLAB Script", "", "MATLAB Files (*.m);;All Files (*.*)");
    if (!fileName.isEmpty()) {
        onFileDoubleClicked(fileName);
    }
}

void MainWindow::onFileDoubleClicked(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file: " + filePath);
        return;
    }

    QTextStream in(&file);
    auto* editor = new CodeEditor(editorTabs_);
    editor->setPlainText(in.readAll());
    file.close();

    QFileInfo info(filePath);
    int idx = editorTabs_->addTab(editor, info.fileName());
    editorTabs_->setCurrentIndex(idx);
}

void MainWindow::onSaveFile() {
    auto* currentEditor = qobject_cast<CodeEditor*>(editorTabs_->currentWidget());
    if (!currentEditor) return;

    QString fileName = QFileDialog::getSaveFileName(this, "Save Script", "", "MATLAB Files (*.m);;All Files (*.*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << currentEditor->toPlainText();
        file.close();

        QFileInfo info(fileName);
        editorTabs_->setTabText(editorTabs_->currentIndex(), info.fileName());
        statusBar()->showMessage("Saved " + info.fileName(), 2000);
    }
}

void MainWindow::onRunScript() {
    auto* currentEditor = qobject_cast<CodeEditor*>(editorTabs_->currentWidget());
    if (!currentEditor) return;

    QString code = currentEditor->toPlainText();
    statusBar()->showMessage("Running script...", 1000);
    onCommandSubmitted(code);
}

void MainWindow::onCommandSubmitted(const QString& command) {
    if (command.trimmed().isEmpty()) {
        return;
    }

    accumulatedCommand_ += command + "\n";

    // Analyze block depth
    matforge::engine::Lexer lexer(accumulatedCommand_.toStdString());
    auto tokens = lexer.tokenize();
    int openBlocks = 0;
    int closeBlocks = 0;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        auto t = tokens[i].type;
        if (t == matforge::engine::TokenType::For ||
            t == matforge::engine::TokenType::While ||
            t == matforge::engine::TokenType::If ||
            t == matforge::engine::TokenType::Function) {
            openBlocks++;
        } else if (t == matforge::engine::TokenType::End) {
            bool isIndexEnd = false;
            if (i > 0) {
                auto prev = tokens[i - 1].type;
                if (prev == matforge::engine::TokenType::LParen ||
                    prev == matforge::engine::TokenType::Colon ||
                    prev == matforge::engine::TokenType::Comma) {
                    isIndexEnd = true;
                }
            }
            if (!isIndexEnd) {
                closeBlocks++;
            }
        }
    }

    if (openBlocks > closeBlocks) {
        // Block is unclosed; wait for closing 'end'
        return;
    }

    QString toExecute = accumulatedCommand_;
    accumulatedCommand_.clear();

    historyWidget_->addCommand(toExecute.trimmed());

    auto result = interpreter_->execute(toExecute.toStdString());
    if (!result.success) {
        commandWindow_->appendError(QString::fromStdString(result.errorMessage));
    } else {
        commandWindow_->appendOutput(QString::fromStdString(result.output));
    }

    workspaceWidget_->refresh();
}

void MainWindow::onVariableInspected(const QString& varName) {
    onCommandSubmitted(varName);
}

void MainWindow::onPlotRequested(const std::vector<double>& x, const std::vector<double>& y, const std::string& title) {
    QMetaObject::invokeMethod(this, [this, x, y, title]() {
        figureWindow_->plot(x, y, QString::fromStdString(title));
    }, Qt::QueuedConnection);
}

} // namespace matforge::gui

