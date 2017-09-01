
#include "graphicsview.h"

graphicsview::graphicsview(QWidget *parent): QGraphicsView(parent)
{
   setMouseTracking(true);
   
   xcen = .5;
   act_xcen = .5;
   ycen = .5;
   act_ycen = .5;
   
   last_scroll_x = -1;
   last_scroll_y = -1;
   
   warning_text = new QTextEdit(this);
   //warning_text->setEnabled(false);
   warning_text->setGeometry(0, 550, 590, 40);
   warning_text->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
   warning_text->viewport()->setAutoFillBackground(false);
   QFont qf = warning_text->currentFont();
   qf.setPointSize(22);
   warning_text->setCurrentFont(qf);
   warning_text->setVisible(false);
   warning_text->setEnabled(false);
   
   warning_text->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   warning_text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   
   //warning_text->setText("Warning!");
   
   loop_text = new QTextEdit(this);
   
   loop_text->setGeometry(370, 2, 180, 35);
   loop_text->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
   loop_text->viewport()->setAutoFillBackground(false);
   qf = loop_text->currentFont();
   qf.setPointSize(18);
   loop_text->setCurrentFont(qf);
   loop_text->setAlignment(Qt::AlignLeft);
   loop_text->setVisible(false);
   loop_text->setEnabled(false);
   
   loop_text->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   loop_text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   
   //loop_text->setText("Warning!");
   
   save_box = new QTextEdit(this);
   
   save_box->setGeometry(1, 1, 30, 35);
   save_box->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
   save_box->viewport()->setAutoFillBackground(false);
   qf = save_box->currentFont();
   qf.setPointSize(18);
   save_box->setCurrentFont(qf);
   save_box->setAlignment(Qt::AlignLeft);
   save_box->setVisible(true);
   save_box->setEnabled(false);
   
   save_box->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   save_box->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   
   save_box->setText("X");
   
   coords = new QTextEdit(this);
   //warning_text->setEnabled(false);
   coords->setGeometry(150, 150, 100, 50);
   coords->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
   coords->viewport()->setAutoFillBackground(false);
   qf = coords->currentFont();
   qf.setPointSize(14);
   coords->setCurrentFont(qf);
   save_box->setAlignment(Qt::AlignHCenter);
   coords->setVisible(false);
   coords->setEnabled(false);
   coords->setTextColor("lime");
   
   coords->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   coords->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   
   
   return;
}

void graphicsview::mouseMoveEvent(QMouseEvent *e)
{
   QPointF mp = get_im_coord(e->pos());
   
   mouse_im_x = mp.x()*nx;
   mouse_im_y = ny-mp.y()*ny;
   
   if(mouse_im_x >= 0 && mouse_im_x <= nx-1 && mouse_im_y >=0 && mouse_im_y <= ny -1)
      emit mouseCoordsChanged();
   
   QGraphicsView::mouseMoveEvent(e);
}

void graphicsview::mousePressEvent(QMouseEvent *e)
{
   QPointF mp;
   
   if(e->button() == Qt::LeftButton)
   {
      mp = get_im_coord(e->pos());
      emit leftPressed(mp);
      //e->ignore();
      //std::cout << "Left Pressed\n";
      QGraphicsView::mousePressEvent(e);
   }
   if(e->button() == Qt::RightButton)
   {
      mp = get_im_coord(e->pos());
      emit rightPressed(mp);
   }
   else QGraphicsView::mousePressEvent(e);
}

QPointF graphicsview::get_im_coord(const QPoint &viewcoord)
{
   double nxcen, nycen, dnx, dny;
   
   dnx = nx;
   dny = ny;
   
   /*nxcen = 300. - (double)viewcoord.x();///((double)width());
    *        nycen = 300. - (double)viewcoord.y();///((double)height());*/
   
   nxcen = .5*((double)width()) - (double)viewcoord.x();///;
   nycen = .5*((double)height()) - (double)viewcoord.y();///;*/
   
   return QPointF((act_xcen*nx-nxcen/(ZoomLevel*ScreenZoom))/dnx, (act_ycen*ny-nycen/(ZoomLevel*ScreenZoom))/dny);
}

void graphicsview::mouseReleaseEvent(QMouseEvent *e)
{
   QPointF mp;
   
   if(e->button()  == Qt::MidButton)
   {
      mp = get_im_coord(e->pos());
      
      //We don't update act_xcen - this must be done after view validation.
      xcen = mp.x();
      ycen = mp.y();
      emit centerChanged();
   }
   
   if(e->button() == Qt::LeftButton)
   {
      mp = get_im_coord(e->pos());
      emit leftClicked(mp);
      QGraphicsView::mouseReleaseEvent(e);
   }
   
   if(e->button() == Qt::RightButton)
   {
      mp = get_im_coord(e->pos());
      emit rightClicked(mp);
   }
   QGraphicsView::mouseReleaseEvent(e);
}

void graphicsview::mouseDoubleClickEvent(QMouseEvent *e)
{
   //std::cout << "Double click\n";
}
void graphicsview::wheelEvent(QWheelEvent *e)
{
   emit wheelMoved(e->delta());
}

