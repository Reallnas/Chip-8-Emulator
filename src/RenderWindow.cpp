#include "RenderWindow.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QWidget>
#include <QPen>

RenderWindow::RenderWindow(QWidget *parent) : QGraphicsScene(parent)
{
    //setSceneRect(0,0,64*m_scale,32*m_scale);
    setBackgroundBrush(Qt::black);
    for(unsigned int i=0;i<64*32;i++)
    {
        const int x = static_cast<int>(i%64) * m_scale;
        const int y = static_cast<int>(i/64) * m_scale;
        m_rect_list[i] = addRect(x,y,m_scale,m_scale,QPen(Qt::black),Qt::black);
    }
}

void RenderWindow::draw(const std::array<u8,64*32>& screen)
{
    for(unsigned int i=0;i<screen.size();i++)
    {
        if(screen[i] == m_previous_screen[i])
        {
            continue;
        }
        if(screen[i] != 0)
        {
            m_rect_list[i]->setBrush(Qt::white);
            m_rect_list[i]->setPen(QPen(Qt::white));
        }
        else
        {
            m_rect_list[i]->setBrush(Qt::black);
            m_rect_list[i]->setPen(QPen(Qt::black));
        }
    }
    m_previous_screen = screen;
}

void RenderWindow::reset()
{
    m_previous_screen = {};
    for(QGraphicsRectItem* rect : m_rect_list)
    {
        rect->setBrush(Qt::black);
        rect->setPen(QPen(Qt::black));
    }
}
