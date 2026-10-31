#pragma once

#include <QPlainTextEdit>
#include <QStringList>

namespace matforge::gui {

class CommandWindow : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CommandWindow(QWidget* parent = nullptr);

    void appendOutput(const QString& text);
    void appendError(const QString& text);
    void clearWindow();

signals:
    void commandSubmitted(const QString& command);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void insertPrompt();
    QString getCurrentCommandLine() const;
    void replaceCurrentCommandLine(const QString& newText);

    QString prompt_{">> "};
    int promptPosition_{0};
    QStringList history_;
    int historyIndex_{-1};
};

} // namespace matforge::gui

