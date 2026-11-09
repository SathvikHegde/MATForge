#pragma once

#include <QWidget>
#include <QPushButton>
#include <QColor>
#include <QPen>
#include <QPointF>
#include <QRect>
#include <QString>
#include <vector>
#include <string>

namespace matforge::gui {

enum class MarkerType {
    None,
    Circle,
    Plus,
    Asterisk,
    Point,
    Cross,
    Square,
    Diamond
};

enum class PlotType {
    Line,
    Bar,
    Stem
};

struct CurveData {
    std::vector<double> x;
    std::vector<double> y;
    QColor color{0, 114, 189};
    Qt::PenStyle lineStyle{Qt::SolidLine};
    int lineWidth{2};
    MarkerType marker{MarkerType::None};
    PlotType plotType{PlotType::Line};
    QString label;
};

class PlotAxes {
public:
    PlotAxes();

    void addCurve(CurveData curve);
    void clear();

    bool holdOn{false};
    bool gridOn{true};
    QString title;
    QString xLabel;
    QString yLabel;
    std::vector<QString> legendLabels;

    // View bounds (for zoom/pan)
    bool autoScale{true};
    double viewMinX{0.0}, viewMaxX{1.0};
    double viewMinY{0.0}, viewMaxY{1.0};

    // Auto-computed data bounds
    double dataMinX{0.0}, dataMaxX{1.0};
    double dataMinY{0.0}, dataMaxY{1.0};

    std::vector<CurveData> curves;

    void updateDataBounds();
    void resetView();
    void render(class QPainter& painter, const QRect& plotBox, const QPoint& hoverPos, bool showCursor);

    [[nodiscard]] QPointF toScreen(double x, double y, const QRect& plotRect) const;
    [[nodiscard]] QPointF toData(double sx, double sy, const QRect& plotRect) const;
    bool findNearestPoint(const QPoint& screenPos, const QRect& plotRect, double& outX, double& outY, QPointF& outScreenPos) const;
};

class FigureCanvas : public QWidget {
    Q_OBJECT

public:
    explicit FigureCanvas(QWidget* parent = nullptr);

    void plot(const std::vector<double>& x, const std::vector<double>& y, 
              const QString& format = "", const QString& plotType = "line", 
              const QString& label = "");
    void setSubplot(int rows, int cols, int index);
    void setHold(bool hold);
    void setGrid(bool grid);
    void setTitle(const QString& title);
    void setXLabel(const QString& label);
    void setYLabel(const QString& label);
    void setLegend(const std::vector<QString>& labels);

    void clearPlot();
    void resetView();
    void exportToImage(const QString& filePath);

    void togglePan(bool enabled) { panEnabled_ = enabled; }
    void toggleDataCursor(bool enabled) { dataCursorEnabled_ = enabled; update(); }
    void toggleGrid();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    PlotAxes& currentAxes();
    const PlotAxes& currentAxes() const;
    QRect getSubplotRect(int r, int c, int totalRows, int totalCols) const;
    int findSubplotAt(const QPoint& pos) const;

    static void parseFormatString(const QString& fmt, QColor& color, Qt::PenStyle& style, MarkerType& marker, bool hasUserColor);
    static QColor getNextColor(std::size_t index);

    int subplotRows_{1};
    int subplotCols_{1};
    int activeSubplotIndex_{0};
    std::vector<PlotAxes> axesList_;

    bool panEnabled_{false};
    bool dataCursorEnabled_{true};
    bool isDragging_{false};
    QPoint lastMousePos_;
    QPoint hoverPos_;
};

class FigureWindow : public QWidget {
    Q_OBJECT

public:
    explicit FigureWindow(QWidget* parent = nullptr);

    FigureCanvas* canvas() { return canvas_; }

    void plot(const std::vector<double>& x, const std::vector<double>& y, 
              const QString& format = "", const QString& plotType = "line");
    void setSubplot(int rows, int cols, int index);
    void setHold(bool hold);
    void setGrid(bool grid);
    void setTitle(const QString& title);
    void setXLabel(const QString& label);
    void setYLabel(const QString& label);
    void setLegend(const std::vector<QString>& labels);
    void clearPlot();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onExportClicked();
    void onClearClicked();
    void onResetViewClicked();
    void onPanToggled(bool checked);
    void onCursorToggled(bool checked);
    void onGridClicked();

private:
    FigureCanvas* canvas_{nullptr};
    QPushButton* panBtn_{nullptr};
    QPushButton* cursorBtn_{nullptr};
    QPushButton* gridBtn_{nullptr};
};

} // namespace matforge::gui
