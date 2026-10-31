#include "matforge/gui/CommandWindow.hpp"

#include <QKeyEvent>
#include <QTextCursor>
#include <QFontDatabase>

namespace matforge::gui {

CommandWindow::CommandWindow(QWidget* parent) : QPlainTextEdit(parent) {
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(10);
    setFont(font);

    setLineWrapMode(QPlainTextEdit::WidgetWidth);

    appendPlainText("MATForge Interactive Console\nMATLAB Clone (C++20 & Qt)\nType 'help' for instructions.\n");
    insertPrompt();
}

void CommandWindow::insertPrompt() {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);

    QTextCharFormat format;
    format.setForeground(QColor(0, 0, 0));
    format.setFontWeight(QFont::Bold);
    cursor.insertText(prompt_, format);

    promptPosition_ = cursor.position();
    ensureCursorVisible();
}

QString CommandWindow::getCurrentCommandLine() const {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.setPosition(promptPosition_, QTextCursor::KeepAnchor);
    return cursor.selectedText();
}

void CommandWindow::replaceCurrentCommandLine(const QString& newText) {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.setPosition(promptPosition_, QTextCursor::KeepAnchor);
    cursor.insertText(newText);
    ensureCursorVisible();
}

void CommandWindow::appendOutput(const QString& text) {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat format;
    format.setForeground(QColor(30, 30, 30));
    cursor.insertText(text, format);
    insertPrompt();
}

void CommandWindow::appendError(const QString& text) {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat format;
    format.setForeground(QColor(200, 20, 20));
    cursor.insertText("Error: " + text + "\n\n", format);
    insertPrompt();
}

void CommandWindow::clearWindow() {
    clear();
    insertPrompt();
}

void CommandWindow::keyPressEvent(QKeyEvent* event) {
    QTextCursor cursor = textCursor();

    if (cursor.position() < promptPosition_ && event->key() != Qt::Key_Up && event->key() != Qt::Key_Down) {
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QString cmd = getCurrentCommandLine().trimmed();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("\n");

        if (!cmd.isEmpty()) {
            history_.append(cmd);
            historyIndex_ = history_.size();
        }

        emit commandSubmitted(cmd);
        return;
    }

    if (event->key() == Qt::Key_Backspace) {
        if (cursor.position() <= promptPosition_) {
            return; // Don't delete past the prompt
        }
    }

    if (event->key() == Qt::Key_Up) {
        if (!history_.isEmpty() && historyIndex_ > 0) {
            historyIndex_--;
            replaceCurrentCommandLine(history_[historyIndex_]);
        }
        return;
    }

    if (event->key() == Qt::Key_Down) {
        if (!history_.isEmpty() && historyIndex_ + 1 < history_.size()) {
            historyIndex_++;
            replaceCurrentCommandLine(history_[historyIndex_]);
        } else if (historyIndex_ + 1 >= history_.size()) {
            historyIndex_ = history_.size();
            replaceCurrentCommandLine("");
        }
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
}

} // namespace matforge::gui

