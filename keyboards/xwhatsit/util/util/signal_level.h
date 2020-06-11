#ifndef SIGNALLEVELMONITORWINDOW_H
#define SIGNALLEVELMONITORWINDOW_H

#include <QDialog>
#include <QGraphicsScene>
#include <QJsonDocument>
#include "hidthread.h"

namespace Ui {
class SignalLevelMonitorWindow;
}

class SignalLevelMonitorWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SignalLevelMonitorWindow(HidThread &thread, QWidget *parent = nullptr);
    ~SignalLevelMonitorWindow();
    void loadLayout(QString name);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void on_layoutSel_activated(const QString &arg1);
    void on_keyboardName(std::string name);
    void on_signallevel(std::vector<uint16_t>);
    void on_reportMonitorError(std::string error_message);

    void on_SignalLevelMonitorWindow_finished(int result);
    void ShowContextMenu(const QPoint &pos);

private:
    Ui::SignalLevelMonitorWindow *ui;
    QGraphicsScene scene;
    QJsonDocument doc;
    const struct kbd_def *keyboard;
    const struct lay_def *current_layout;
    HidThread &thread;
    std::vector<std::vector<uint16_t>> signal_level;
    std::vector<std::vector<uint16_t>> min_signal_level;
    std::vector<std::vector<uint16_t>> max_signal_level;
    QColor getColor(uint16_t value, uint16_t mins, uint16_t maxs);
    void displaySquare(int x, int y, int w, int h, unsigned int col, unsigned int row, uint16_t mins, uint16_t maxs, QPainter &painter);
    void updateCurrentLayout();
    void setMinimumSizeUnits(unsigned int width_units_times_8, unsigned int height_units_times_8);

    unsigned int keyboard_width_uis_times_8;
    unsigned int keyboard_height_uis_times_8;
    static constexpr double MIN_HORIZONTAL_SCALE = 32;
    static constexpr double MIN_VERTICAL_SCALE = 42;
    static const int HORIZONTAL_MARGIN = 10;
    static const int VERTICAL_MARGIN = 10;
};

#endif // SIGNALLEVELMONITORWINDOW_H
