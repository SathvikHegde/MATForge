#include "matforge/gui/MatlabHighlighter.hpp"
#include <QColor>
#include <QFont>

namespace matforge::gui {

MatlabHighlighter::MatlabHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    // Keywords
    keywordFormat_.setForeground(QColor(0, 0, 255)); // Blue
    keywordFormat_.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bfunction\\b"), QStringLiteral("\\bend\\b"),
        QStringLiteral("\\bif\\b"), QStringLiteral("\\belse\\b"), QStringLiteral("\\belseif\\b"),
        QStringLiteral("\\bfor\\b"), QStringLiteral("\\bwhile\\b"),
        QStringLiteral("\\bswitch\\b"), QStringLiteral("\\bcase\\b"), QStringLiteral("\\botherwise\\b"),
        QStringLiteral("\\btry\\b"), QStringLiteral("\\bcatch\\b"),
        QStringLiteral("\\breturn\\b"), QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bcontinue\\b"),
        QStringLiteral("\\bglobal\\b"), QStringLiteral("\\bpersistent\\b")
    };
    for (const auto& pattern : keywordPatterns) {
        rules_.push_back({QRegularExpression(pattern), keywordFormat_});
    }

    // Built-ins
    builtinFormat_.setForeground(QColor(128, 0, 128)); // Purple
    const QString builtinPatterns[] = {
        QStringLiteral("\\bsin\\b"), QStringLiteral("\\bcos\\b"), QStringLiteral("\\btan\\b"),
        QStringLiteral("\\bsqrt\\b"), QStringLiteral("\\bexp\\b"), QStringLiteral("\\blog\\b"),
        QStringLiteral("\\babs\\b"), QStringLiteral("\\bzeros\\b"), QStringLiteral("\\bones\\b"),
        QStringLiteral("\\beye\\b"), QStringLiteral("\\blinspace\\b"), QStringLiteral("\\bdet\\b"),
        QStringLiteral("\\btrace\\b"), QStringLiteral("\\bsum\\b"), QStringLiteral("\\bmean\\b"),
        QStringLiteral("\\bmin\\b"), QStringLiteral("\\bmax\\b"), QStringLiteral("\\bsize\\b"),
        QStringLiteral("\\blength\\b"), QStringLiteral("\\bdisp\\b"), QStringLiteral("\\bplot\\b"),
        QStringLiteral("\\bclear\\b"), QStringLiteral("\\bclc\\b"), QStringLiteral("\\bwho\\b"),
        QStringLiteral("\\bwhos\\b"), QStringLiteral("\\bhelp\\b")
    };
    for (const auto& pattern : builtinPatterns) {
        rules_.push_back({QRegularExpression(pattern), builtinFormat_});
    }

    // Numbers
    numberFormat_.setForeground(QColor(180, 100, 0)); // Dark Orange
    rules_.push_back({QRegularExpression(QStringLiteral("\\b[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?\\b")), numberFormat_});

    // Strings
    stringFormat_.setForeground(QColor(163, 21, 21)); // Dark Red
    rules_.push_back({QRegularExpression(QStringLiteral("'[^'\n]*'")), stringFormat_});
    rules_.push_back({QRegularExpression(QStringLiteral("\"[^\"\n]*\"")), stringFormat_});

    // Single-line comments (%)
    commentFormat_.setForeground(QColor(34, 139, 34)); // Forest Green
    commentFormat_.setFontItalic(true);
    rules_.push_back({QRegularExpression(QStringLiteral("%.*")), commentFormat_});
}

void MatlabHighlighter::highlightBlock(const QString& text) {
    for (const auto& rule : rules_) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

} // namespace matforge::gui

