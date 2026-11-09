#include "matforge/gui/FigureWindow.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QToolTip>
#include <algorithm>
#include <cmath>

namespace matforge::gui {

namespace {
const std::vector<QColor> kDefaultPalette = {
    QColor(0, 114, 189),   // Blue
    QColor(217, 83, 25),   // Orange
    QColor(237, 177, 32),  // Yellow
    QColor(126, 47, 142),  // Purple
    QColor(119, 172, 48),  // Green
    QColor(77, 190, 238),  // Cyan
    QColor(162, 20, 47)    // Red
};
}

PlotAxes::PlotAxes() = default;

void PlotAxes::addCurve(CurveData curve) {
    if (!holdOn) {
        curves.clear();
    }
    curves.push_back(std::move(curve));
    updateDataBounds();
    if (autoScale) {
        resetView();
    }
}

void PlotAxes::clear() {
    curves.clear();
    title.clear();
    xLabel.clear();
    yLabel.clear();
    legendLabels.clear();
    autoScale = true;
    updateDataBounds();
    resetView();
}

void PlotAxes::updateDataBounds() {
    if (curves.empty()) {
        dataMinX = 0.0; dataMaxX = 1.0;
        dataMinY = 0.0; dataMaxY = 1.0;
        return;
    }

    bool first = true;
    for (const auto& c : curves) {
        for (std::size_t i = 0; i < c.x.size() && i < c.y.size(); ++i) {
            double x = c.x[i];
            double y = c.y[i];
            if (first) {
                dataMinX = dataMaxX = x;
                dataMinY = dataMaxY = y;
                first = false;
            } else {
                dataMinX = std::min(dataMinX, x);
                dataMaxX = std::max(dataMaxX, x);
                dataMinY = std::min(dataMinY, y);
                dataMaxY = std::max(dataMaxY, y);
            }
        }
    }

    if (std::abs(dataMaxX - dataMinX) < 1e-9) { dataMinX -= 1.0; dataMaxX += 1.0; }
    if (std::abs(dataMaxY - dataMinY) < 1e-9) { dataMinY -= 1.0; dataMaxY += 1.0; }
}

void PlotAxes::resetView() {
    viewMinX = dataMinX;
    viewMaxX = dataMaxX;
    viewMinY = dataMinY;
    viewMaxY = dataMaxY;
    autoScale = true;
}

QPointF PlotAxes::toScreen(double x, double y, const QRect& plotRect) const {
    double minX = autoScale ? dataMinX : viewMinX;
    double maxX = autoScale ? dataMaxX : viewMaxX;
    double minY = autoScale ? dataMinY : viewMinY;
    double maxY = autoScale ? dataMaxY : viewMaxY;

    double normX = (x - minX) / (maxX - minX);
    double normY = (y - minY) / (maxY - minY);
    double sx = plotRect.left() + normX * plotRect.width();
    double sy = plotRect.bottom() - normY * plotRect.height();
    return QPointF(sx, sy);
}

QPointF PlotAxes::toData(double sx, double sy, const QRect& plotRect) const {
    double minX = autoScale ? dataMinX : viewMinX;
    double maxX = autoScale ? dataMaxX : viewMaxX;
    double minY = autoScale ? dataMinY : viewMinY;
    double maxY = autoScale ? dataMaxY : viewMaxY;

    double normX = (sx - plotRect.left()) / static_cast<double>(plotRect.width());
    double normY = (plotRect.bottom() - sy) / static_cast<double>(plotRect.height());
    double dx = minX + normX * (maxX - minX);
    double dy = minY + normY * (maxY - minY);
    return QPointF(dx, dy);
}

bool PlotAxes::findNearestPoint(const QPoint& screenPos, const QRect& plotRect, double& outX, double& outY, QPointF& outScreenPos) const {
    if (!plotRect.contains(screenPos) || curves.empty()) return false;

    double minDistSq = 2500.0; // within ~50 pixels
    bool found = false;

    for (const auto& c : curves) {
        for (std::size_t i = 0; i < c.x.size() && i < c.y.size(); ++i) {
            QPointF sp = toScreen(c.x[i], c.y[i], plotRect);
            double dx = sp.x() - screenPos.x();
            double dy = sp.y() - screenPos.y();
            double distSq = dx * dx + dy * dy;
            if (distSq < minDistSq) {
                minDistSq = distSq;
                outX = c.x[i];
                outY = c.y[i];
                outScreenPos = sp;
                found = true;
            }
        }
    }
    return found;
}

void PlotAxes::render(QPainter& painter, const QRect& box, const QPoint& hoverPos, bool showCursor) {
    const int leftMargin = 65;
    const int rightMargin = 25;
    const int topMargin = 35;
    const int bottomMargin = 45;

    QRect plotRect(box.left() + leftMargin, box.top() + topMargin,
                    box.width() - leftMargin - rightMargin,
                    box.height() - topMargin - bottomMargin);

    if (plotRect.width() <= 10 || plotRect.height() <= 10) return;

    // Background & Box
    painter.fillRect(plotRect, Qt::white);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(plotRect);

    // Title
    if (!title.isEmpty()) {
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.setPen(Qt::black);
        painter.drawText(QRect(box.left(), box.top() + 5, box.width(), 25), Qt::AlignCenter, title);
    }

    // X Label
    if (!xLabel.isEmpty()) {
        painter.setFont(QFont("Arial", 9, QFont::Normal));
        painter.setPen(Qt::black);
        painter.drawText(QRect(plotRect.left(), plotRect.bottom() + 25, plotRect.width(), 20), Qt::AlignCenter, xLabel);
    }

    // Y Label
    if (!yLabel.isEmpty()) {
        painter.save();
        painter.setFont(QFont("Arial", 9, QFont::Normal));
        painter.setPen(Qt::black);
        painter.translate(box.left() + 15, plotRect.center().y());
        painter.rotate(-90);
        painter.drawText(QRect(-plotRect.height() / 2, -15, plotRect.height(), 20), Qt::AlignCenter, yLabel);
        painter.restore();
    }

    if (curves.empty()) {
        painter.setFont(QFont("Arial", 9));
        painter.setPen(Qt::gray);
        painter.drawText(plotRect, Qt::AlignCenter, "No data plotted");
        return;
    }

    double minX = autoScale ? dataMinX : viewMinX;
    double maxX = autoScale ? dataMaxX : viewMaxX;
    double minY = autoScale ? dataMinY : viewMinY;
    double maxY = autoScale ? dataMaxY : viewMaxY;

    // Grid lines & Ticks
    const int numTicks = 5;
    painter.setFont(QFont("Arial", 8));

    for (int i = 0; i <= numTicks; ++i) {
        double frac = static_cast<double>(i) / numTicks;

        // Vertical Grid
        int px = plotRect.left() + static_cast<int>(frac * plotRect.width());
        if (gridOn) {
            painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::DashLine));
            painter.drawLine(px, plotRect.top(), px, plotRect.bottom());
        }
        painter.setPen(Qt::black);
        painter.drawLine(px, plotRect.bottom(), px, plotRect.bottom() + 4);
        double valX = minX + frac * (maxX - minX);
        painter.drawText(px - 30, plotRect.bottom() + 6, 60, 16, Qt::AlignCenter, QString::number(valX, 'g', 4));

        // Horizontal Grid
        int py = plotRect.bottom() - static_cast<int>(frac * plotRect.height());
        if (gridOn) {
            painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::DashLine));
            painter.drawLine(plotRect.left(), py, plotRect.right(), py);
        }
        painter.setPen(Qt::black);
        painter.drawLine(plotRect.left() - 4, py, plotRect.left(), py);
        double valY = minY + frac * (maxY - minY);
        painter.drawText(box.left() + 5, py - 8, leftMargin - 12, 16, Qt::AlignRight | Qt::AlignVCenter, QString::number(valY, 'g', 4));
    }

    // Clip to plot box for drawing data
    painter.save();
    painter.setClipRect(plotRect);

    for (const auto& curve : curves) {
        if (curve.x.empty()) continue;

        if (curve.plotType == PlotType::Bar) {
            // Bar plot
            double barWidth = (plotRect.width() / static_cast<double>(curve.x.size())) * 0.7;
            barWidth = std::max(2.0, std::min(30.0, barWidth));
            painter.setPen(QPen(curve.color.darker(120), 1));
            painter.setBrush(QBrush(curve.color));

            for (std::size_t i = 0; i < curve.x.size() && i < curve.y.size(); ++i) {
                QPointF topPt = toScreen(curve.x[i], curve.y[i], plotRect);
                QPointF basePt = toScreen(curve.x[i], 0.0, plotRect);
                double rx = topPt.x() - barWidth / 2.0;
                double ry = std::min(topPt.y(), basePt.y());
                double rh = std::abs(topPt.y() - basePt.y());
                painter.drawRect(QRectF(rx, ry, barWidth, rh));
            }
        } else if (curve.plotType == PlotType::Stem) {
            // Stem plot: vertical line + circle at head
            painter.setPen(QPen(curve.color, curve.lineWidth));
            painter.setBrush(QBrush(curve.color));
            for (std::size_t i = 0; i < curve.x.size() && i < curve.y.size(); ++i) {
                QPointF topPt = toScreen(curve.x[i], curve.y[i], plotRect);
                QPointF basePt = toScreen(curve.x[i], 0.0, plotRect);
                painter.drawLine(basePt, topPt);
                painter.drawEllipse(topPt, 3.5, 3.5);
            }
        } else {
            // Line plot
            if (curve.lineStyle != Qt::NoPen) {
                QPen linePen(curve.color, curve.lineWidth, curve.lineStyle);
                painter.setPen(linePen);
                for (std::size_t i = 0; i + 1 < curve.x.size() && i + 1 < curve.y.size(); ++i) {
                    QPointF p1 = toScreen(curve.x[i], curve.y[i], plotRect);
                    QPointF p2 = toScreen(curve.x[i + 1], curve.y[i + 1], plotRect);
                    painter.drawLine(p1, p2);
                }
            }

            // Draw markers if specified
            if (curve.marker != MarkerType::None) {
                painter.setPen(QPen(curve.color, 1.5));
                painter.setBrush(Qt::NoBrush);
                const double mSize = 3.5;

                for (std::size_t i = 0; i < curve.x.size() && i < curve.y.size(); ++i) {
                    QPointF pt = toScreen(curve.x[i], curve.y[i], plotRect);
                    switch (curve.marker) {
                        case MarkerType::Circle:
                            painter.drawEllipse(pt, mSize, mSize);
                            break;
                        case MarkerType::Square:
                            painter.drawRect(QRectF(pt.x() - mSize, pt.y() - mSize, mSize * 2, mSize * 2));
                            break;
                        case MarkerType::Diamond: {
                            QPointF pts[4] = {
                                QPointF(pt.x(), pt.y() - mSize),
                                QPointF(pt.x() + mSize, pt.y()),
                                QPointF(pt.x(), pt.y() + mSize),
                                QPointF(pt.x() - mSize, pt.y())
                            };
                            painter.drawPolygon(pts, 4);
                            break;
                        }
                        case MarkerType::Plus:
                            painter.drawLine(QPointF(pt.x() - mSize, pt.y()), QPointF(pt.x() + mSize, pt.y()));
                            painter.drawLine(QPointF(pt.x(), pt.y() - mSize), QPointF(pt.x(), pt.y() + mSize));
                            break;
                        case MarkerType::Cross:
                            painter.drawLine(QPointF(pt.x() - mSize, pt.y() - mSize), QPointF(pt.x() + mSize, pt.y() + mSize));
                            painter.drawLine(QPointF(pt.x() - mSize, pt.y() + mSize), QPointF(pt.x() + mSize, pt.y() - mSize));
                            break;
                        case MarkerType::Asterisk:
                            painter.drawLine(QPointF(pt.x() - mSize, pt.y()), QPointF(pt.x() + mSize, pt.y()));
                            painter.drawLine(QPointF(pt.x(), pt.y() - mSize), QPointF(pt.x(), pt.y() + mSize));
                            painter.drawLine(QPointF(pt.x() - mSize * 0.7, pt.y() - mSize * 0.7), QPointF(pt.x() + mSize * 0.7, pt.y() + mSize * 0.7));
                            painter.drawLine(QPointF(pt.x() - mSize * 0.7, pt.y() + mSize * 0.7), QPointF(pt.x() + mSize * 0.7, pt.y() - mSize * 0.7));
                            break;
                        case MarkerType::Point:
                            painter.fillRect(QRectF(pt.x() - 1, pt.y() - 1, 2, 2), curve.color);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    painter.restore();

    // Draw Legend if labels exist
    std::vector<QString> activeLegend = legendLabels;
    if (activeLegend.empty()) {
        for (const auto& c : curves) {
            if (!c.label.isEmpty()) activeLegend.push_back(c.label);
        }
    }

    if (!activeLegend.empty()) {
        painter.setFont(QFont("Arial", 8));
        int legWidth = 90;
        for (const auto& l : activeLegend) {
            legWidth = std::max(legWidth, painter.fontMetrics().horizontalAdvance(l) + 40);
        }
        int legHeight = static_cast<int>(activeLegend.size()) * 18 + 8;
        QRect legRect(plotRect.right() - legWidth - 8, plotRect.top() + 8, legWidth, legHeight);

        painter.fillRect(legRect, QColor(255, 255, 255, 220));
        painter.setPen(QPen(QColor(180, 180, 180), 1));
        painter.drawRect(legRect);

        for (std::size_t i = 0; i < activeLegend.size(); ++i) {
            int ly = legRect.top() + 12 + static_cast<int>(i) * 18;
            QColor curColor = (i < curves.size()) ? curves[i].color : kDefaultPalette[i % kDefaultPalette.size()];
            painter.setPen(QPen(curColor, 2));
            painter.drawLine(legRect.left() + 8, ly, legRect.left() + 24, ly);

            painter.setPen(Qt::black);
            painter.drawText(legRect.left() + 28, ly + 4, activeLegend[i]);
        }
    }

    // Data Cursor Tooltip
    if (showCursor) {
        double nearX, nearY;
        QPointF screenPt;
        if (findNearestPoint(hoverPos, plotRect, nearX, nearY, screenPt)) {
            painter.setPen(QPen(Qt::red, 1.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(screenPt, 5.0, 5.0);
            painter.drawLine(QPointF(screenPt.x() - 8, screenPt.y()), QPointF(screenPt.x() + 8, screenPt.y()));
            painter.drawLine(QPointF(screenPt.x(), screenPt.y() - 8), QPointF(screenPt.x(), screenPt.y() + 8));

            QString tip = QString("X: %1\nY: %2").arg(QString::number(nearX, 'g', 5)).arg(QString::number(nearY, 'g', 5));
            painter.setFont(QFont("Arial", 8, QFont::Bold));
            QRect tipBox(static_cast<int>(screenPt.x()) + 10, static_cast<int>(screenPt.y()) - 25, 90, 32);
            painter.fillRect(tipBox, QColor(255, 255, 225, 230));
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(tipBox);
            painter.drawText(tipBox.adjusted(4, 2, -4, -2), Qt::AlignLeft | Qt::AlignVCenter, tip);
        }
    }
}

// =========================================================================
// FigureCanvas
// =========================================================================

FigureCanvas::FigureCanvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(450, 320);
    setStyleSheet("background-color: white;");
    setMouseTracking(true);
    axesList_.emplace_back(); // single main axes by default
}

PlotAxes& FigureCanvas::currentAxes() {
    if (activeSubplotIndex_ >= static_cast<int>(axesList_.size())) {
        axesList_.resize(activeSubplotIndex_ + 1);
    }
    return axesList_[activeSubplotIndex_];
}

const PlotAxes& FigureCanvas::currentAxes() const {
    if (activeSubplotIndex_ < static_cast<int>(axesList_.size())) {
        return axesList_[activeSubplotIndex_];
    }
    static PlotAxes defaultAxes;
    return defaultAxes;
}

QColor FigureCanvas::getNextColor(std::size_t index) {
    return kDefaultPalette[index % kDefaultPalette.size()];
}

void FigureCanvas::parseFormatString(const QString& fmt, QColor& color, Qt::PenStyle& style, MarkerType& marker, bool hasUserColor) {
    if (fmt.isEmpty()) return;

    // Check color
    if (!hasUserColor) {
        if (fmt.contains('r')) color = QColor(217, 83, 25);
        else if (fmt.contains('g')) color = QColor(119, 172, 48);
        else if (fmt.contains('b')) color = QColor(0, 114, 189);
        else if (fmt.contains('c')) color = QColor(77, 190, 238);
        else if (fmt.contains('m')) color = QColor(126, 47, 142);
        else if (fmt.contains('y')) color = QColor(237, 177, 32);
        else if (fmt.contains('k')) color = Qt::black;
        else if (fmt.contains('w')) color = Qt::white;
    }

    // Check line style
    if (fmt.contains("--")) style = Qt::DashLine;
    else if (fmt.contains("-.")) style = Qt::DashDotLine;
    else if (fmt.contains(':')) style = Qt::DotLine;
    else if (fmt.contains('-')) style = Qt::SolidLine;

    // Check marker
    bool hasMarker = false;
    if (fmt.contains('o')) { marker = MarkerType::Circle; hasMarker = true; }
    else if (fmt.contains('+')) { marker = MarkerType::Plus; hasMarker = true; }
    else if (fmt.contains('*')) { marker = MarkerType::Asterisk; hasMarker = true; }
    else if (fmt.contains('.')) { marker = MarkerType::Point; hasMarker = true; }
    else if (fmt.contains('x')) { marker = MarkerType::Cross; hasMarker = true; }
    else if (fmt.contains('s')) { marker = MarkerType::Square; hasMarker = true; }
    else if (fmt.contains('d')) { marker = MarkerType::Diamond; hasMarker = true; }

    // If marker specified without explicit line style, suppress line
    if (hasMarker && !fmt.contains('-') && !fmt.contains(':')) {
        style = Qt::NoPen;
    }
}

void FigureCanvas::plot(const std::vector<double>& x, const std::vector<double>& y, 
                        const QString& format, const QString& plotType, const QString& label) {
    CurveData curve;
    curve.x = x;
    curve.y = y;
    curve.label = label;

    auto& ax = currentAxes();
    curve.color = getNextColor(ax.curves.size());

    if (plotType == "bar") {
        curve.plotType = PlotType::Bar;
    } else if (plotType == "stem") {
        curve.plotType = PlotType::Stem;
    } else {
        curve.plotType = PlotType::Line;
    }

    if (!format.isEmpty()) {
        parseFormatString(format, curve.color, curve.lineStyle, curve.marker, false);
    }

    ax.addCurve(std::move(curve));
    update();
}

void FigureCanvas::setSubplot(int rows, int cols, int index) {
    if (rows < 1 || cols < 1 || index < 1) return;
    subplotRows_ = rows;
    subplotCols_ = cols;
    activeSubplotIndex_ = index - 1; // 1-based to 0-based

    std::size_t total = static_cast<std::size_t>(rows * cols);
    if (axesList_.size() < total) {
        axesList_.resize(total);
    }
    update();
}

void FigureCanvas::setHold(bool hold) {
    currentAxes().holdOn = hold;
}

void FigureCanvas::setGrid(bool grid) {
    currentAxes().gridOn = grid;
    update();
}

void FigureCanvas::toggleGrid() {
    currentAxes().gridOn = !currentAxes().gridOn;
    update();
}

void FigureCanvas::setTitle(const QString& title) {
    currentAxes().title = title;
    update();
}

void FigureCanvas::setXLabel(const QString& label) {
    currentAxes().xLabel = label;
    update();
}

void FigureCanvas::setYLabel(const QString& label) {
    currentAxes().yLabel = label;
    update();
}

void FigureCanvas::setLegend(const std::vector<QString>& labels) {
    currentAxes().legendLabels = labels;
    update();
}

void FigureCanvas::clearPlot() {
    for (auto& ax : axesList_) {
        ax.clear();
    }
    subplotRows_ = 1;
    subplotCols_ = 1;
    activeSubplotIndex_ = 0;
    axesList_.resize(1);
    update();
}

void FigureCanvas::resetView() {
    for (auto& ax : axesList_) {
        ax.resetView();
    }
    update();
}

void FigureCanvas::exportToImage(const QString& filePath) {
    QPixmap pixmap(size());
    render(&pixmap);
    pixmap.save(filePath);
}

QRect FigureCanvas::getSubplotRect(int r, int c, int totalRows, int totalCols) const {
    int w = width() / totalCols;
    int h = height() / totalRows;
    return QRect(c * w, r * h, w, h);
}

int FigureCanvas::findSubplotAt(const QPoint& pos) const {
    int col = pos.x() / (width() / subplotCols_);
    int row = pos.y() / (height() / subplotRows_);
    col = std::max(0, std::min(subplotCols_ - 1, col));
    row = std::max(0, std::min(subplotRows_ - 1, row));
    return row * subplotCols_ + col;
}

void FigureCanvas::paintEvent(QPaintEvent* /* event */) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    for (int r = 0; r < subplotRows_; ++r) {
        for (int c = 0; c < subplotCols_; ++c) {
            int idx = r * subplotCols_ + c;
            if (idx < static_cast<int>(axesList_.size())) {
                QRect subBox = getSubplotRect(r, c, subplotRows_, subplotCols_);
                axesList_[idx].render(painter, subBox, hoverPos_, dataCursorEnabled_);
            }
        }
    }
}

void FigureCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        int idx = findSubplotAt(event->pos());
        if (idx < static_cast<int>(axesList_.size())) {
            activeSubplotIndex_ = idx;
        }
        isDragging_ = true;
        lastMousePos_ = event->pos();
    }
}

void FigureCanvas::mouseMoveEvent(QMouseEvent* event) {
    hoverPos_ = event->pos();

    if (isDragging_ && (panEnabled_ || (event->buttons() & Qt::RightButton))) {
        QPoint delta = event->pos() - lastMousePos_;
        lastMousePos_ = event->pos();

        auto& ax = currentAxes();
        int r = activeSubplotIndex_ / subplotCols_;
        int c = activeSubplotIndex_ % subplotCols_;
        QRect subBox = getSubplotRect(r, c, subplotRows_, subplotCols_);

        double spanX = (ax.autoScale ? (ax.dataMaxX - ax.dataMinX) : (ax.viewMaxX - ax.viewMinX));
        double spanY = (ax.autoScale ? (ax.dataMaxY - ax.dataMinY) : (ax.viewMaxY - ax.viewMinY));

        double dx = -delta.x() * (spanX / static_cast<double>(subBox.width()));
        double dy = delta.y() * (spanY / static_cast<double>(subBox.height()));

        if (ax.autoScale) {
            ax.viewMinX = ax.dataMinX; ax.viewMaxX = ax.dataMaxX;
            ax.viewMinY = ax.dataMinY; ax.viewMaxY = ax.dataMaxY;
            ax.autoScale = false;
        }

        ax.viewMinX += dx; ax.viewMaxX += dx;
        ax.viewMinY += dy; ax.viewMaxY += dy;
        update();
        return;
    }

    if (dataCursorEnabled_) {
        update();
    }
}

void FigureCanvas::mouseReleaseEvent(QMouseEvent* /* event */) {
    isDragging_ = false;
}

void FigureCanvas::wheelEvent(QWheelEvent* event) {
    int idx = findSubplotAt(event->position().toPoint());
    if (idx < static_cast<int>(axesList_.size())) {
        activeSubplotIndex_ = idx;
    }

    auto& ax = currentAxes();
    if (ax.autoScale) {
        ax.viewMinX = ax.dataMinX; ax.viewMaxX = ax.dataMaxX;
        ax.viewMinY = ax.dataMinY; ax.viewMaxY = ax.dataMaxY;
        ax.autoScale = false;
    }

    double zoomFactor = (event->angleDelta().y() > 0) ? 0.85 : 1.15;

    int r = activeSubplotIndex_ / subplotCols_;
    int c = activeSubplotIndex_ % subplotCols_;
    QRect subBox = getSubplotRect(r, c, subplotRows_, subplotCols_);
    QRect plotRect = subBox.adjusted(65, 35, -25, -45);

    QPointF cursorData = ax.toData(event->position().x(), event->position().y(), plotRect);

    ax.viewMinX = cursorData.x() + (ax.viewMinX - cursorData.x()) * zoomFactor;
    ax.viewMaxX = cursorData.x() + (ax.viewMaxX - cursorData.x()) * zoomFactor;
    ax.viewMinY = cursorData.y() + (ax.viewMinY - cursorData.y()) * zoomFactor;
    ax.viewMaxY = cursorData.y() + (ax.viewMaxY - cursorData.y()) * zoomFactor;

    update();
}

void FigureCanvas::mouseDoubleClickEvent(QMouseEvent* /* event */) {
    currentAxes().resetView();
    update();
}

// =========================================================================
// FigureWindow
// =========================================================================

FigureWindow::FigureWindow(QWidget* parent)
    : QWidget(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint) {
    setWindowTitle("MATForge - Figure");
    resize(700, 500);
    setAttribute(Qt::WA_QuitOnClose, false);

    auto* layout = new QVBoxLayout(this);
    canvas_ = new FigureCanvas(this);
    layout->addWidget(canvas_);

    auto* btnLayout = new QHBoxLayout();
    panBtn_ = new QPushButton("Pan", this);
    panBtn_->setCheckable(true);
    cursorBtn_ = new QPushButton("Data Cursor", this);
    cursorBtn_->setCheckable(true);
    cursorBtn_->setChecked(true);
    gridBtn_ = new QPushButton("Grid", this);
    auto* resetBtn = new QPushButton("Reset View", this);
    auto* exportBtn = new QPushButton("Export Image...", this);
    auto* clearBtn = new QPushButton("Clear", this);
    auto* closeBtn = new QPushButton("Close", this);

    btnLayout->addWidget(panBtn_);
    btnLayout->addWidget(cursorBtn_);
    btnLayout->addWidget(gridBtn_);
    btnLayout->addWidget(resetBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(clearBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(panBtn_, &QPushButton::toggled, this, &FigureWindow::onPanToggled);
    connect(cursorBtn_, &QPushButton::toggled, this, &FigureWindow::onCursorToggled);
    connect(gridBtn_, &QPushButton::clicked, this, &FigureWindow::onGridClicked);
    connect(resetBtn, &QPushButton::clicked, this, &FigureWindow::onResetViewClicked);
    connect(exportBtn, &QPushButton::clicked, this, &FigureWindow::onExportClicked);
    connect(clearBtn, &QPushButton::clicked, this, &FigureWindow::onClearClicked);
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
}

void FigureWindow::plot(const std::vector<double>& x, const std::vector<double>& y, 
                        const QString& format, const QString& plotType) {
    canvas_->plot(x, y, format, plotType);
    show();
    raise();
    activateWindow();
}

void FigureWindow::setSubplot(int rows, int cols, int index) {
    canvas_->setSubplot(rows, cols, index);
    show();
    raise();
}

void FigureWindow::setHold(bool hold) {
    canvas_->setHold(hold);
}

void FigureWindow::setGrid(bool grid) {
    canvas_->setGrid(grid);
}

void FigureWindow::setTitle(const QString& title) {
    canvas_->setTitle(title);
}

void FigureWindow::setXLabel(const QString& label) {
    canvas_->setXLabel(label);
}

void FigureWindow::setYLabel(const QString& label) {
    canvas_->setYLabel(label);
}

void FigureWindow::setLegend(const std::vector<QString>& labels) {
    canvas_->setLegend(labels);
}

void FigureWindow::clearPlot() {
    canvas_->clearPlot();
}

void FigureWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }
    QWidget::keyPressEvent(event);
}

void FigureWindow::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export Figure", "figure.png", "PNG Images (*.png);;JPEG Images (*.jpg);;BMP Images (*.bmp)");
    if (!fileName.isEmpty()) {
        canvas_->exportToImage(fileName);
    }
}

void FigureWindow::onClearClicked() {
    clearPlot();
}

void FigureWindow::onResetViewClicked() {
    canvas_->resetView();
}

void FigureWindow::onPanToggled(bool checked) {
    canvas_->togglePan(checked);
}

void FigureWindow::onCursorToggled(bool checked) {
    canvas_->toggleDataCursor(checked);
}

void FigureWindow::onGridClicked() {
    canvas_->toggleGrid();
}

} // namespace matforge::gui
