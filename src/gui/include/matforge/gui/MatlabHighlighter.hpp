#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <vector>

namespace matforge::gui {

class MatlabHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit MatlabHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    std::vector<HighlightingRule> rules_;
    QTextCharFormat keywordFormat_;
    QTextCharFormat builtinFormat_;
    QTextCharFormat numberFormat_;
    QTextCharFormat stringFormat_;
    QTextCharFormat commentFormat_;
};

} // namespace matforge::gui

