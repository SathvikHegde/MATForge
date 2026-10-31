#pragma once

#include <QWidget>
#include <vector>
#include <string>

namespace matforge::gui {

class FigureCanvas : public QWidget {
    Q_OBJECT

public:
    explicit FigureCanvas(QWidget* parent = nullptr);

    void setPlotData(const std::vector<double>& x, const std::vector<double>& y, const QString& title = "Figure 1");
    void clearPlot();
    void exportToImage(const QString& filePath);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<double> xData_;
    std::vector<double> yData_;
    QString title_{"Figure 1"};
    bool hasData_{false};
};

class FigureWindow : public QWidget {
    Q_OBJECT

public:
    explicit FigureWindow(QWidget* parent = nullptr);

    void plot(const std::vector<double>& x, const std::vector<double>& y, const QString& title = "Figure 1");

private slots:
    void onExportClicked();
    void onClearClicked();

private:
    FigureCanvas* canvas_{nullptr};
};

} // namespace matforge::gui

