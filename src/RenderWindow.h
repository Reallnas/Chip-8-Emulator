#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <array>
#include <QGraphicsScene>

class QWidget;
class QGraphicsRectItem;

using u8 = unsigned char;

class RenderWindow : public QGraphicsScene
{
public:
    RenderWindow(QWidget *parent = nullptr);

    void draw(const std::array<u8,64*32>& screen);
    void reset();

private:
    int m_scale = 15;
    std::array<u8,64*32> m_previous_screen {};
    std::array<QGraphicsRectItem*,64*32> m_rect_list {};
};

#endif // RENDERWINDOW_H
