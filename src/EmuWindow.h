#ifndef EMUWINDOW_H
#define EMUWINDOW_H

#include <QWidget>
#include <QGraphicsView>
#include "Chip8.h"

class RenderWindow;
class QTimer;

class EmuWindow : public QGraphicsView
{
    Q_OBJECT
public:
    explicit EmuWindow(QWidget *parent = nullptr);
    void startEmulation(const char* file_name);
    void stopEmulation();
    void setEmulationSpeed(const int frequency);

private:
    void connectSignals();

    void switchEmulationState();
    void updateFrame();
    void updateDelayAndSound();
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;

    RenderWindow* m_render_window;
    QTimer* m_frame_timer;
    QTimer* m_delay_and_sound_timer;

    Chip8 m_chip8;
    bool m_is_emulation_running = false;
};

#endif // EMUWINDOW_H
