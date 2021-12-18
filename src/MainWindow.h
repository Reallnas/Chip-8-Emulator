#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <string>
#include <QMainWindow>

#include "EmuWindow.h"

class QMenu;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow();
    virtual ~MainWindow() override;

private:
    void chooseFile();

    QMenu* m_file_menu;
    QMenu* m_emulation_menu;

    //File
    QAction* m_open_file_action;
    QAction* m_close_action;
    QAction* m_quit_action;

    //Emulation
    QMenu* m_change_speed_menu;
    EmuWindow* m_emu_window;
};

#endif // MAINWINDOW_H
