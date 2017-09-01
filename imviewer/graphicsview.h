#ifndef __graphicsview_h__
#define __graphicsview_h__

#include <QGraphicsView>
#include <QScrollBar>
#include <QMouseEvent>

#include <QTextEdit>

#include <iostream>

class graphicsview : public QGraphicsView
{
   Q_OBJECT
   
   public:
      graphicsview(QWidget *parent = 0);
      
      double xcen;
      double act_xcen;
      double ycen;
      double act_ycen;
      
      double mouse_im_x;
      double mouse_im_y;
      
      
      int last_scroll_x;
      int last_scroll_y;
      
      int nx;
      int ny;
      
      double ZoomLevel;
      double ScreenZoom;
      
      QPointF get_im_coord(const QPoint &viewcoord);
      
      QTextEdit * warning_text;
      QTextEdit * loop_text;
      QTextEdit * save_box;
      QTextEdit * coords;
      
   signals:
      void centerChanged();
      void mouseCoordsChanged();
      void leftPressed(QPointF mp);
      void leftClicked(QPointF mp);
      void rightPressed(QPointF mp);
      void rightClicked(QPointF mp);
      void wheelMoved(int delta);
   
   protected:
      void mouseMoveEvent(QMouseEvent *e);
      void mousePressEvent(QMouseEvent *e);
      void mouseReleaseEvent(QMouseEvent *e);
      void mouseDoubleClickEvent(QMouseEvent *e);
      
      void wheelEvent(QWheelEvent *e);
      
};

#endif //__graphicsview_h__

