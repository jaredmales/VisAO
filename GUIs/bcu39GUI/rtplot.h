
#ifndef __rtPlotForm_h__
#define __rtPlotForm_h__

#include <QDialog>
#include <QTimer>

#include "ui_rtplot.h"

#include <qwt_data.h>
#include <qwt_plot_curve.h>
#include <iostream>

class circleTimeSeries : public QwtData
{
   public:
      circleTimeSeries(size_t sz, bool useage)
      {
         set_length(sz);
         cursor = -1;
         _size = 0;
         useAge = useage;
         minY = 1e99;
         maxY = 0;
      }

      virtual QwtData * copy() const;

      std::vector<double> t;
      std::vector<double> Y;
      std::vector<double> age;
      
      void set_length(size_t l)
      {
         t.resize(l);
         Y.resize(l);
         age.resize(l);
      }
      size_t get_length() const { return t.size();}
      
      int cursor;
      size_t _size;
      virtual size_t size() const{ return _size;}
      
      double minY;
      double maxY;
      
      bool useAge;
      
      void add_point(double nt, double ny)
      {
         double poppedY;
         double dt;
         size_t _i;
         
         if(_size < get_length())
         {
            t[_size] = nt;
            Y[_size] = ny;
            age[_size] = 0;
            if(_size > 0)
            {
               dt = t[_size] - t[_size-1];
               for(size_t i=0;i<_size;i++) age[i] += dt;
            }
            
            _size++;
            cursor++;
            if(cursor >= get_length()) cursor = 0;
            
            if(ny < minY) minY = ny;
            if(ny > maxY) maxY = ny;
         }
         else
         {
            size_t next = cursor+1;
            if(next >= get_length()) next = 0;

            poppedY = Y[next];
            
            t[next] = nt;
            Y[next] = ny;
            
            dt = t[next] - t[cursor];
            
            cursor = next;
            age[cursor] = 0;
            for(size_t i=0;i<get_length();i++) age[sample_i(i)] += dt;

            if(poppedY == maxY)
            {
               maxY = 0;
               for(size_t i=0;i<get_length();i++)
               {
                  _i = sample_i(i);
                  if(Y[_i] > maxY) maxY = Y[_i] ;
               }
            }
            if(poppedY == minY)
            {
               minY = 1e99;
               for(size_t i=0;i<get_length();i++)
               {
                  _i = sample_i(i);
                  if(Y[_i] < minY) minY = Y[_i] ;
               }
            }
         }
      }

      size_t sample_i(size_t i) const
      {
         size_t _i = cursor + 1 + i;
         if(_i >= _size) _i -= _size;
         return _i;
      }

      void clear()
      {
         cursor = -1;
         _size = 0;
         minY = 1e99;
         maxY = 0;
      }
      
      virtual double x(size_t i) const
      {
         if(useAge) return age[sample_i(i)];
         else return t[sample_i(i)];
      }

      virtual double y(size_t i) const
      {
         return Y[sample_i(i)];
      }

      
      virtual QRectF boundingRect() const
      {
         if(useAge)
         {
            //std::cout << _size << " " << get_length() << " " << age[sample_i(0)] << "\n";
            /*if(_size == 10)
            {
               for(size_t i =0;i<10;i++)
               {
                  std::cout << sample_i(i) << " " << t[i] << " " << age[i] << " " << Y[i] << "\n";
               }
               exit(0);
            }*/
            return QRectF(0, minY, age[sample_i(0)], maxY-minY);
         }
         else
         {
            return QRectF(t[cursor], minY, t[cursor]-t[sample_i(0)], maxY-minY);
         }   
      }
};

   
class rtPlotForm : public QDialog
{
   Q_OBJECT
   
   public:
      rtPlotForm(QString title, QString ylabel, size_t sz, bool useage, int pause, QWidget * Parent = 0, Qt::WindowFlags f = 0);
      ~rtPlotForm();
      
      int statsPause;

      QTimer updateTimer; ///< When this times out the GUI is updated.
      int updateTimerTimeout;
      

      QwtPlotCurve * curve;
      
      circleTimeSeries *data;

      void reset();
      
   protected slots:
      void updatePlot();
      void on_clearButton_pressed();
      void on_replotButton_pressed();
      
   private:
      Ui::rtPlotForm ui;
   
};

#endif //__rtPlotForm_h__
