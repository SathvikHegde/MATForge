#include "matforge/gui/FigureWindow.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <algorithm>
#include <cmath>

namespace matforge::gui {

FigureCanvas::FigureCanvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    setStyleSheet("background-color: white;");
}

void FigureCanvas::setPlotData(const std::vector<double>& x, const std::vector<double>& y, const QString& title) {
    xData_ = x;
    yData_ = y;
    title_ = title;
    hasData_ = (!xData_.empty() && xData_.size() == yData_.size());
    update();
}

void FigureCanvas::clearPlot() {
    xData_.clear();
    yData_.clear();
    hasData_ = false;
    update();
}

void FigureCanvas::exportToImage(const QString& filePath) {
    QPixmap pixmap(size());
    render(&pixmap);
    pixmap.save(filePath);
}

void FigureCanvas::paintEvent(QPaintEvent* /* event */) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Canvas background
    painter.fillRect(rect(), Qt::white);

    // Margins for axes
    const int leftMargin = 60;
    const int rightMargin = 30;
    const int topMargin = 40;
    const int bottomMargin = 50;

    QRect plotRect(leftMargin, topMargin, 
                    width() - leftMargin - rightMargin, 
                    height() - topMargin - bottomMargin);

    // Draw Plot Box
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(plotRect);

    // Title
    painter.setFont(QFont("Arial", 11, QFont::Bold));
    painter.drawText(QRect(0, 10, width(), 25), Qt::AlignCenter, title_);

    if (!hasData_) {
        painter.setFont(QFont("Arial", 10, QFont::Normal));
        painter.setPen(Qt::gray);
        painter.drawText(plotRect, Qt::AlignCenter, "No data plotted.\nUse plot(x, y) in the Command Window.");
        return;
    }

    // Determine data ranges
    double minX = *std::min_element(xData_.begin(), xData_.end());
    double maxX = *std::max_element(xData_.begin(), xData_.end());
    double minY = *std::min_element(yData_.begin(), yData_.end());
    double maxY = *std::max_element(yData_.begin(), yData_.end());

    if (std::abs(maxX - minX) < 1e-9) { minX -= 1.0; maxX += 1.0; }
    if (std::abs(maxY - minY) < 1e-9) { minY -= 1.0; maxY += 1.0; }

    // Draw Grid Lines (5 divisions)
    painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::DashLine));
    painter.setFont(QFont("Arial", 8));
    const int numTicks = 5;

    for (int i = 0; i <= numTicks; ++i) {
        // Vertical grid & X tick labels
        double fracX = static_cast<double>(i) / numTicks;
        int px = plotRect.left() + static_cast<int>(fracX * plotRect.width());
        painter.drawLine(px, plotRect.top(), px, plotRect.bottom());

        double valX = minX + fracX * (maxX - minX);
        painter.setPen(Qt::black);
        painter.drawText(px - 25, plotRect.bottom() + 5, 50, 20, Qt::AlignCenter, QString::number(valX, 'g', 3));
        painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::DashLine));

        // Horizontal grid & Y tick labels
        double fracY = static_cast<double>(i) / numTicks;
        int py = plotRect.bottom() - static_cast<int>(fracY * plotRect.height());
        painter.drawLine(plotRect.left(), py, plotRect.right(), py);

        double valY = minY + fracY * (maxY - minY);
        painter.setPen(Qt::black);
        painter.drawText(plotRect.left() - 55, py - 10, 50, 20, Qt::AlignRight | Qt::AlignVCenter, QString::number(valY, 'g', 3));
        painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::DashLine));
    }

    // Draw Plot Line
    QPen linePen(QColor(0, 114, 189), 2); // Classic MATLAB default blue
    painter.setPen(linePen);

    auto toScreenPoint = [&](double x, double y) -> QPointF {
        double normX = (x - minX) / (maxX - minX);
        double normY = (y - minY) / (maxY - minY);
        double sx = plotRect.left() + normX * plotRect.width();
        double sy = plotRect.bottom() - normY * plotRect.height();
        return QPointF(sx, sy);
    };

    for (std::size_t i = 0; i + 1 < xData_.size(); ++i) {
        QPointF p1 = toScreenPoint(xData_[i], yData_[i]);
        QPointF p2 = toScreenPoint(xData_[i + 1], yData_[i + 1]);
        painter.drawLine(p1, p2);
    }
}

FigureWindow::FigureWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("MATForge - Figure");
    resize(640, 480);

    auto* layout = new QVBoxLayout(this);
    canvas_ = new FigureCanvas(this);
    layout->addWidget(canvas_);

    auto* btnLayout = new QHBoxLayout();
    auto* exportBtn = new QPushButton("Export Image...", this);
    auto* clearBtn = new QPushButton("Clear", this);

    btnLayout->addStretch();
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(clearBtn);
    layout->addLayout(btnLayout);

    connect(exportBtn, &QPushButton::clicked, this, &FigureWindow::onExportClicked);
    connect(clearBtn, &QPushButton::clicked, this, &FigureWindow::onClearClicked);
}

void FigureWindow::plot(const std::vector<double>& x, const std::vector<double>& y, const QString& title) {
    canvas_->setPlotData(x, y, title);
    show();
    raise();
    activateWindow();
}

void FigureWindow::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export Plot", "", "PNG Image (*.png);;BMP Image (*.bmp)");
    if (!fileName.isEmpty()) {
        canvas_->exportToImage(fileName);
    }
}

void FigureWindow::onClearClicked() {
    canvas_->clearPlot();
}

} // namespace matforge::gui

