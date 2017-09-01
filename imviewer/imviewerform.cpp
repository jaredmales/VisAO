#include "imviewerform.h"

imviewerForm::imviewerForm(int data_type, imviewer_shmt shkey, QWidget * Parent, Qt::WindowFlags f) : imviewer(data_type, shkey, Parent, f)
{
   ui.setupUi(this);
   nup =0;
   
   imcp = 0;
   pointerOverZoom = 4.;
   
   defHeight = 550;
   defWidth = 550;
   //This will come up at some minimal size.
   ui.graphicsView->setGeometry(0,0, defHeight, defWidth);
   
   qgs = new QGraphicsScene();
   ui.graphicsView->setScene(qgs);
   
   qpmi = 0;
   userBox = 0;
   statsBox = 0;
   
   IgnoreXYSliderChange = false;
   
   rightClickDragging = false;
   
   NullMouseCoords = true;
   
   
   set_mindat(400);
   
   set_maxdat(600);
   

   
   //launchControlPanel();
#if RT_SYSTEM == RT_SYSTEM_VISAO
   statusboard_shmemptr = 0;
   
   //Init the status board
   int statusboard_shmemkey = STATUS_imviewer;
   int rv;
   size_t sizecheck;
   
   rv = create_shmem(&statusboard_shmemid, statusboard_shmemkey, sizeof(VisAO::imviewer_status_board));
   
   if(!rv)
   {
      //std::cout << "Status board created\n";
      statusboard_shmemptr = attach_shm(&sizecheck,  statusboard_shmemkey, statusboard_shmemid);
      if(statusboard_shmemptr) 
      {
         //std::cout << "Status board attached\n";
         rv = 0;
      }
      else rv = -1;
   }
   
   if(rv < 0)
   {
      statusboard_shmemptr = 0;
   }
   else
   {
      VisAO::basic_status_board * bsb = (VisAO::basic_status_board *) statusboard_shmemptr;
      strncpy(bsb->appname, "imviewer", 25);
      bsb->max_update_interval = 100000;
   }
#endif

   userBox_i0 = 0;
   userBox_i1 = 32;
   userBox_j0 = 0;
   userBox_j1 = 32;
   userBox = new StretchBox(0,0,32,32);
   userBox->setPen(QPen("Yellow"));
   userBox->setVisible(false);
   userBox->setStretchable(true);
   connect(userBox, SIGNAL(moved(const QRectF & )), this, SLOT(userBoxMoved(const QRectF & )));
   connect(userBox, SIGNAL(rejectMouse()), this, SLOT(userBoxRejectMouse()));
   
   qgs->addItem(userBox);

   statsBox = new StretchBox(0,0,32,32);
   statsBox->setPen(QPen("Red"));
   statsBox->setVisible(false);
   statsBox->setStretchable(true);
   connect(statsBox, SIGNAL(moved(const QRectF & )), this, SLOT(statsBoxMoved(const QRectF & )));
   connect(statsBox, SIGNAL(rejectMouse()), this, SLOT(statsBoxRejectMouse()));
   
   qgs->addItem(statsBox);

   
   guideBox = new StretchBox(0,0,32,32);
   guideBox->setPen(QPen("Cyan"));
   guideBox->setVisible(false);
   guideBox->setStretchable(true);
   connect(guideBox, SIGNAL(moved(const QRectF & )), this, SLOT(guideBoxMoved(const QRectF & )));
   connect(guideBox, SIGNAL(rejectMouse()), this, SLOT(guideBoxRejectMouse()));
   
   qgs->addItem(guideBox);

   userCircle = new StretchCircle(512,512, 64, 64);
   userCircle->setPen(QPen("lime"));
   userCircle->setStretchable(true);
   userCircle->setVisible(false);
   connect(userCircle, SIGNAL(resized(const float &)), this, SLOT(userCircleResized(const float &)));
   connect(userCircle, SIGNAL(moved(const QRectF & )), this, SLOT(userCircleMoved(const QRectF & )));
   connect(userCircle, SIGNAL(mouseIn()), this, SLOT(userCircleMouseIn()));
   connect(userCircle, SIGNAL(mouseOut()), this, SLOT(userCircleMouseOut()));
   connect(userCircle, SIGNAL(rejectMouse()), this, SLOT(userCircleRejectMouse()));
   
   qgs->addItem(userCircle);
   
   
   targetVisible = false;
   
   cenLineVert = 0;//qgs->addLine(QLineF(.5*getNx(),0, .5*getNx(), getNy()), QColor("lime"));
   cenLineHorz = 0;//qgs->addLine(QLineF(0, .5*getNy(), getNx(), .5*getNy()), QColor("lime"));
   
   imStats = 0;
   
   setImsize(1024,1024); //Just for initial setup.   

   if(shmem_attached) timerout(); //Get first image

   imtimer.start(imtimer_timeout); //and set timer.
#if RT_SYSTEM == RT_SYSTEM_VISAO  
   useAOStatus = 1;
   aosb = 0; //Initialize the pointer to the AO system status board.
#endif

   nup = qgs->addLine(QLineF(512,400, 512, 624), QColor("lime"));
   nup_tip = qgs->addLine(QLineF(512,400, 536, 424), QColor("lime"));
   nup->setTransformOriginPoint ( QPointF(512,512) );
   nup_tip->setTransformOriginPoint ( QPointF(512,512) );

   QPen qp = nup->pen();
   qp.setWidth(5);

   nup->setPen(qp);
   nup_tip->setPen(qp);
   
   
      
}

void imviewerForm::postSetImsize()
{
   ui.xSlider->setMaximum(nx*2);
   ui.ySlider->setMaximum(ny*2);
   
   ui.graphicsView->xcen = .5;
   ui.graphicsView->ycen = .5;
   
   ScreenZoom = (float)defWidth/(float)nx;
   
   if(imcp)
   {
      QTransform transform;
      float viewZoom = (float)imcp->ui.viewView->width()/(float)nx;
      
      transform.scale(viewZoom, viewZoom);
      imcp->ui.viewView->setTransform(transform);
   }
   
   set_ZoomLevel(1.0);

   //Resize the user color box
   userBox_i0 = ny*.25;
   userBox_i1 = ny*.75;

   userBox_j0 = nx*.25;
   userBox_j1 = nx*.75;

   //std::cout << userBox_i0 << " " << userBox_i1 - userBox_i0 << " " << userBox_j0 << " " << userBox_j1 - userBox_j0<< "\n";
   userBox->setRect(userBox->mapRectFromScene(userBox_i0, userBox_j0, userBox_i1-userBox_i0, userBox_j1-userBox_j0));
   userBox->setEdgeTol(5./ScreenZoom < 5 ? 5 : 5./ScreenZoom);

   //resize the stats box
   statsBox->setRect(statsBox->mapRectFromScene(ny*.25, nx*.3, .4*ny, .4*nx));
   statsBox->setEdgeTol(5./ScreenZoom < 5 ? 5 : 5./ScreenZoom);
   statsBoxMoved(statsBox->rect());
   
   //resize the guide box
   guideBox->setRect(statsBox->mapRectFromScene(ny*.3, nx*.3, .4*ny, .4*nx));
   guideBox->setEdgeTol(5./ScreenZoom < 5 ? 5 : 5./ScreenZoom);
   guideBoxMoved(guideBox->rect());
   
   //resize the circle
   userCircle->setRect(userCircle->mapRectFromScene(ny*.35, nx*.35, .4*ny, .4*nx));
   userCircle->setEdgeTol(5./ScreenZoom < 5 ? 5 : 5./ScreenZoom);
   userCircleMoved(guideBox->rect());
   
   if(!cenLineVert)
   {
      cenLineVert = qgs->addLine(QLineF(.5*getNx(),0, .5*getNx(), getNy()), QColor("lime"));
      cenLineHorz = qgs->addLine(QLineF(0, .5*getNy(), getNx(), .5*getNy()), QColor("lime"));
      if(targetVisible)
      {
         cenLineVert->setVisible(true);
         cenLineHorz->setVisible(true);
      }
      else
      {
         cenLineVert->setVisible(false);
         cenLineHorz->setVisible(false);
      } 
   }
   else
   {
      cenLineVert->setLine(QLineF(.5*getNx(),0, .5*getNx(), getNy()));
      cenLineHorz->setLine(QLineF(0, .5*getNy(), getNx(), .5*getNy()));
   }
}

void imviewerForm::post_set_ZoomLevel()
{
   QTransform transform;
   
   ui.graphicsView->nx = nx;
   ui.graphicsView->ny = ny;
   ui.graphicsView->ZoomLevel = ZoomLevel;
   ui.graphicsView->ScreenZoom = ScreenZoom;
   
   transform.scale(ZoomLevel*ScreenZoom, ZoomLevel*ScreenZoom);
   
   ui.graphicsView->setTransform(transform);
   transform.scale(pointerOverZoom, pointerOverZoom);
   if(imcp) imcp->ui.pointerView->setTransform(transform);
   change_center();
   
   if(nup)
   {
      nup->setLine(ui.graphicsView->act_xcen*nx, ui.graphicsView->act_ycen*ny-.1*ny/ZoomLevel, ui.graphicsView->act_xcen*nx, ui.graphicsView->act_ycen*ny+.1*ny/ZoomLevel);
      nup->setTransformOriginPoint ( QPointF(ui.graphicsView->act_xcen*nx,ui.graphicsView->act_ycen*ny) );
         
      nup_tip->setLine(QLineF(ui.graphicsView->act_xcen*nx,ui.graphicsView->act_ycen*ny-.1*ny/ZoomLevel, ui.graphicsView->act_xcen*nx + .02*nx/ZoomLevel,ui.graphicsView->act_ycen*ny-.1*ny/ZoomLevel + .012*ny/ZoomLevel));
      nup_tip->setTransformOriginPoint (  QPointF(ui.graphicsView->act_xcen*nx,ui.graphicsView->act_ycen*ny) );

      QPen qp = nup->pen();
   
      float wid = 5/(ZoomLevel*ScreenZoom);
      if(wid > 3) wid = 3;
      //if(wid < 1) wid = 1;
      //std::cout << wid << "\n";
      qp.setWidth(wid);

      nup->setPen(qp);
      nup_tip->setPen(qp);
   }
  
}

void imviewerForm::postChangeImdata()
{
   char strtmp[20];
   
   sprintf(strtmp, "%6.3f", fps_ave);
   
   ui.fpsGage->setText(strtmp);
   
  
   if(saturated)
   {
      ui.graphicsView->warning_text->setText("Saturated!");
      ui.graphicsView->warning_text->setTextColor(warning_color);
      ui.graphicsView->warning_text->setTextBackgroundColor(QColor(0,0,0,0));
      ui.graphicsView->warning_text->setVisible(true);
      //if(userBox) ui.graphicsView->stackUnder(userBox);
      //if(statsBox) ui.graphicsView->stackUnder(statsBox);
   }
   else
   {
      ui.graphicsView->warning_text->setVisible(false);
   }
   
   if(!qpmi) qpmi = qgs->addPixmap(qpm);//QPixmap::fromImage(*qim));
   else qpmi->setPixmap(qpm);
        
   if(userBox) qpmi->stackBefore(userBox);
   if(statsBox) qpmi->stackBefore(statsBox);
   if(guideBox) qpmi->stackBefore(guideBox);
   
   
   
//    else
//    {
      
//    if(targetVisible)
//    {
//       qpmi->stackBefore(cenLineVert);
//       qpmi->stackBefore(cenLineHorz);
//    }
   
   if(imcp)
   {
      if(imcp->ViewViewMode == ViewViewEnabled)
      {
         if(!imcp->qpmi_view) imcp->qpmi_view = imcp->qgs_view->addPixmap(qpm);
         imcp->qpmi_view->setPixmap(qpm);
         
         imcp->qpmi_view->stackBefore(imcp->viewLineVert);
      }
   }
   update_MouseCoords(); //This is to update the pixel val box if set.
   
   if(imcp)
   {
      imcp->update_panel();
   }
   
#if RT_SYSTEM == RT_SYSTEM_VISAO
   if(imStats) 
   {
      if(applyDark && dark_sim.imdata) imStats->set_imdata(imdata, frame_time, dark_sim.imdata);
      else  imStats->set_imdata(imdata, frame_time,0);
   }
#endif

#if RT_SYSTEM == RT_SYSTEM_SCEXAO
   if(imStats) 
   {
      imStats->set_imdata(imdata, frame_time,0);
   }
#endif

#if RT_SYSTEM == RT_SYSTEM_VISAO
   size_t sz;
   if(!aosb && useAOStatus) aosb = (VisAO::aosystem_status_board*) attach_shm(&sz,  STATUS_aosystem, 0);
   
   if(aosb && useAOStatus)
   {
      //std::cout << -aosb->pa << "\n";
      float sgn = -1;
      nup->setRotation(aosb->rotoffset+90);//(aosb->rotang-aosb->el)+sgn*aosb->pa);
      nup_tip->setRotation(aosb->rotoffset+90);//(aosb->rotang-aosb->el)+sgn*aosb->pa);
   }
   
#endif

   if(applyDarkChanged)
   {
      applyDarkChanged = 0;
      on_buttonReStretch_clicked();
      
   }
}

void imviewerForm::launchControlPanel()
{
   if(!imcp)
   {
      imcp = new imviewerControlPanel(this, Qt::Tool);
      connect(imcp, SIGNAL(launchStatsBox()), this, SLOT(doLaunchStatsBox()));
      connect(imcp, SIGNAL(hideStatsBox()), this, SLOT(doHideStatsBox()));
   }
   
   imcp->show();
   
   imcp->activateWindow();
}



void imviewerForm::on_buttonPanelLaunch_clicked()
{
   launchControlPanel();
}

void imviewerForm::on_buttonFreezeRealTime_clicked()
{
   if(RealTimeStopped)
   {
      set_RealTimeStopped(false);
      ui.buttonFreezeRealTime->setText("Stop");
   }
   else
   {
      set_RealTimeStopped(true);
      ui.buttonFreezeRealTime->setText("Start");
      ui.fpsGage->setText("0.000");
   }
}

void imviewerForm::on_buttonReStretch_clicked()
{
   
   if(get_abs_fixed())
   {
      if(get_colorbar_mode() == user)
      {
         set_colorbar_mode(minmaxglobal);
      }
      
      if(get_colorbar_mode() == minmaxglobal)
      {
         set_mindat(get_imdat_min());
         set_maxdat(get_imdat_max());
         changeImdata(false);
      }

      if(get_colorbar_mode() == minmaxbox)
      {
         set_mindat(userBox_min);
         set_maxdat(userBox_max);
         changeImdata(false);
      }
   }
}

void imviewerForm::on_darkSubCheckBox_stateChanged(int st)
{
   if(st == 0)
   {
      applyDark = 0;
      applyDarkChanged = 1;
   }
   else
   {
      applyDark = 1;
      applyDarkChanged = 1;
      //std::cout << dark_sim.imdata << std::endl;
      
   }
}


float imviewerForm::get_act_xcen()
{
   return ui.graphicsView->act_xcen;
}

float imviewerForm::get_act_ycen()
{
   return ui.graphicsView->act_ycen;
}

void imviewerForm::setPointerOverZoom(float poz)
{
   pointerOverZoom = poz;
   post_set_ZoomLevel();
}



void imviewerForm::change_center(bool movezoombox)
{
   float nxcen, nycen;
   float vszX, vszY;
   
   //Here we check to see if the requested center point can be realized based
   //on the zoom level.  Without these checks the centerOn() function changes
   //the zoom.
   //If the requested xcen, ycen would cause these problems, we adjust act_xcen, act_ycen
   //This only matters on the edges.
   
   //First validate
   if(ui.graphicsView->xcen < 0) ui.graphicsView->xcen = 0;
   if(ui.graphicsView->xcen > 1.) ui.graphicsView->xcen = 1.;
   
   if(ui.graphicsView->ycen < 0) ui.graphicsView->ycen = 0;
   if(ui.graphicsView->ycen > 1.) ui.graphicsView->ycen = 1.;
   
   vszX = ((float)nx)/ZoomLevel; //The zoomed x size of the image
   
   nxcen = ui.graphicsView->xcen*nx; //The requested x-pixel center
   
   if(nxcen - .5*vszX <  0) nxcen = .5*vszX; //If we're off the left edge
        if(nxcen + .5*vszX > nx) nxcen = nx - .5*vszX; //If we're off the right edge

        ui.graphicsView->act_xcen = nxcen / (float) nx;
   
   vszY = ((float)ny)/ZoomLevel;//The zoomed y size of the image
   
   nycen = ui.graphicsView->ycen*ny;//The requested y-pixel center
   
   if(nycen - .5*vszY <  0) nycen = .5*vszY; //If we're off the top edge
        if(nycen + .5*vszY > ny) nycen = ny - .5*vszY;//If we're off the bottom edge

        ui.graphicsView->act_ycen = nycen / (float) ny;
   
   ui.graphicsView->centerOn(nxcen, nycen);

   //std::cout << nxcen << " " << nycen << "\n";
   
   update_XYsliders();
   
   if(imcp)
   {
      
      imcp->viewLineVert->setLine(ui.graphicsView->xcen*nx, 0, ui.graphicsView->xcen*nx, ny);
      imcp->viewLineHorz->setLine(0, ui.graphicsView->ycen*ny, nx, ui.graphicsView->ycen*ny);
      
      if(ZoomLevel <= 1.0) imcp->viewBox->setVisible(false);
      else
      {
         imcp->viewBox->setVisible(true);
         if(movezoombox)
         {
            QPointF tmpp = imcp->viewBox->mapFromParent(ui.graphicsView->act_xcen*nx - .5*nx/ZoomLevel, ui.graphicsView->act_ycen*ny-.5*ny/ZoomLevel);
            imcp->viewBox->setRect(tmpp.x(), tmpp.y(), nx/ZoomLevel, ny/ZoomLevel);
            //imcp->viewBox->xoff = tmpp.x();
            //imcp->viewBox->yoff = tmpp.y();
         }
         
      }
      imcp->ui.viewView->centerOn(.5*nx, .5*ny);
      imcp->update_panel();
   }
   
}

void imviewerForm::set_viewcen(float x, float y, bool movezoombox)
{
   ui.graphicsView->xcen = x;
   ui.graphicsView->ycen = y;
   change_center(movezoombox);
}

void imviewerForm::update_XYsliders()
{
   IgnoreXYSliderChange = true;
   ui.xSlider->setSliderPosition((int)(ui.graphicsView->xcen*ui.xSlider->maximum()+.5));
   ui.ySlider->setSliderPosition((int)(ui.ySlider->maximum()-ui.graphicsView->ycen*ui.ySlider->maximum()+.5));
   IgnoreXYSliderChange = false;
}

void imviewerForm::on_xSlider_valueChanged(int value)
{
   if(!IgnoreXYSliderChange)
   {
      ui.graphicsView->xcen = (float)value/ui.xSlider->maximum();
      change_center();
   }
}

void imviewerForm::on_ySlider_valueChanged(int value)
{
   if(!IgnoreXYSliderChange)
   {
      ui.graphicsView->ycen = (float)(ui.ySlider->maximum()-value)/ui.ySlider->maximum();
      change_center();
   }
}

void imviewerForm::changeCenter()
{
   change_center();
}

void imviewerForm::resizeEvent(QResizeEvent *)
{
   change_center();
   ui.graphicsView->warning_text->setGeometry(0, ui.graphicsView->height()-50, 590, 40);
}

void imviewerForm::mouseMoveEvent(QMouseEvent *e)
{
   nullMouseCoords();
}

void imviewerForm::nullMouseCoords()
{
   if(!NullMouseCoords)
   {
      if(imcp)
      {
         imcp->nullMouseCoords();
      }
      
      NullMouseCoords = true;
      ui.textCoordX->setText("");
      ui.textCoordY->setText("");
      ui.textPixelVal->setText("");
   }
}

void imviewerForm::update_MouseCoords()
{
   char tmpr[10];

   int idx_x, idx_y;
   
   if(!imdata) return;
   
   if(!NullMouseCoords)
   {
      sprintf(tmpr, "%.1f", ui.graphicsView->mouse_im_x);
      ui.textCoordX->setText(tmpr);
      
      sprintf(tmpr, "%.1f", ui.graphicsView->mouse_im_y);
      ui.textCoordY->setText(tmpr);
      
      idx_x = ((int)(ui.graphicsView->mouse_im_x));
      if(idx_x < 0) idx_x = 0;
      if(idx_x > nx-1) idx_x = nx-1;
      idx_y = (int)(ui.graphicsView->mouse_im_y);
      if(idx_y < 0) idx_y = 0;
      if(idx_y > ny-1) idx_y = ny-1;

#if RT_SYSTEM == RT_SYSTEM_VISAO        
      if(!applyDark || !dark_sim.imdata)
      {
         sprintf(tmpr, "%.1f", pixget(imdata,(int)(idx_y*ny) + (int)(idx_x)) );
      }
      else
      {
         sprintf(tmpr, "%.1f", pixget(imdata,(int)(idx_y*ny) + (int)(idx_x) ) - pixget(dark_sim.imdata,(int)(idx_y*ny) + (int)(idx_x) ));
      }
#else
      sprintf(tmpr, "%.1f", pixget(imdata, (int)(idx_y*ny) + (int)(idx_x)) );
#endif
      
      ui.textPixelVal->setText(tmpr);
      
      if(imcp)
      {
         #if RT_SYSTEM == RT_SYSTEM_VISAO        
         if(!applyDark)
         {
            imcp->updateMouseCoords(ui.graphicsView->mouse_im_x, ui.graphicsView->mouse_im_y, pixget(imdata,idx_y*ny + idx_x ));
         }
         else
         {
            imcp->updateMouseCoords(ui.graphicsView->mouse_im_x, ui.graphicsView->mouse_im_y, ( pixget(imdata,idx_y*ny + idx_x)- pixget(dark_sim.imdata,(int)(idx_y*ny) + (int)(idx_x)) ));
         }
         #else
         imcp->updateMouseCoords(ui.graphicsView->mouse_im_x, ui.graphicsView->mouse_im_y, pixget(imdata,idx_y*ny + idx_x) );
         #endif

      }
   }
   
   if(rightClickDragging)
   {
      float dx = ui.graphicsView->mouse_im_x - rightClickStart.x()*nx;
      float dy = ui.graphicsView->mouse_im_y - rightClickStart.y()*ny;
      
      float dbias = dx/(nx/ZoomLevel);
      float dcontrast = -1.*dy/(ny/ZoomLevel);
      
      set_bias(biasStart + .5*dbias*.5*(imdat_max+imdat_min));
      set_contrast(contrastStart + dcontrast*.5*(imdat_max-imdat_min));
      if(!amChangingimdata) changeImdata();
   }
}

void imviewerForm::changeMouseCoords()
{
   NullMouseCoords = false;
   update_MouseCoords();
}

void imviewerForm::viewLeftPressed(QPointF mp)
{
   if(imcp)
   {
      imcp->viewLeftPressed(mp);
   }
}

void imviewerForm::viewLeftClicked(QPointF mp)
{
   if(imcp)
   {
      imcp->viewLeftClicked(mp);
   }
}

void imviewerForm::viewRightPressed(QPointF mp)
{
   rightClickDragging = true;
   rightClickStart = mp;
   biasStart = get_bias();
   contrastStart = get_contrast();
   
}

void imviewerForm::viewRightClicked(QPointF mp)
{
   rightClickDragging = false;   
}

void imviewerForm::onWheelMoved(int delta)
{
   float dz;
   //std::cout << delta << "\n";
   if(delta > 0)   dz = 1.41421;//*delta/120.;
   else dz = 0.70711;//*delta/120.;
   
   set_ZoomLevel(dz*get_ZoomLevel());
   
}


void imviewerForm::stale_fps()
{
   char fpstmp[20];
   sprintf(fpstmp, "%6.3f", fps_ave);
   ui.fpsGage->setText(fpstmp);
      
}

void imviewerForm::update_age()
{
   char fpstmp[10];
   struct timeval tvtmp;
   gettimeofday(&tvtmp, 0);
   double timetmp = (double)tvtmp.tv_sec + ((double)tvtmp.tv_usec)/1e6;
   snprintf(fpstmp, 7, "%6.2f", timetmp-fps_time0);
   ui.ageGage->setText(fpstmp);
   
#if RT_SYSTEM == RT_SYSTEM_VISAO
   if(aosb && useAOStatus)
   {
      if(aosb->loop_on == 0)
      { 
         ui.graphicsView->loop_text->setText("Loop OPEN");
         ui.graphicsView->loop_text->setTextColor(QColor("red"));
         ui.graphicsView->loop_text->setTextBackgroundColor(QColor(0,0,0,0));
         ui.graphicsView->loop_text->setVisible(true);
      }
      if(aosb->loop_on == 2)
      { 
         ui.graphicsView->loop_text->setText("Loop PAUSED");
         ui.graphicsView->loop_text->setTextColor(QColor("yellow"));
         ui.graphicsView->loop_text->setTextBackgroundColor(QColor(0,0,0,0));
         ui.graphicsView->loop_text->setVisible(true);
      }
      if(aosb->loop_on == 1)
      { 
         ui.graphicsView->loop_text->setText("Loop CLOSED");
         ui.graphicsView->loop_text->setTextColor(QColor("lime"));
         ui.graphicsView->loop_text->setTextBackgroundColor(QColor(0,0,0,0));
         ui.graphicsView->loop_text->setVisible(true);
      }  
  }
  else
  {
      ui.graphicsView->loop_text->setVisible(false);
  }
#else
   ui.graphicsView->loop_text->setVisible(false);
#endif
  
  if(curr_saved == 1)
  {
     ui.graphicsView->save_box->setText("S");
     ui.graphicsView->save_box->setTextColor(QColor("lime"));
     ui.graphicsView->save_box->setVisible(true);
  }
  else
  {
     ui.graphicsView->save_box->setText("X");
     ui.graphicsView->save_box->setTextColor(QColor("red"));
     ui.graphicsView->save_box->setVisible(true);
  }   
  
}

void imviewerForm::on_expandButton_clicked()
{
   static int called = 0;
   int dsize = 55;
   if(!called)
   {
      ui.buttonReStretch->hide();
      ui.buttonFreezeRealTime->hide();
      ui.buttonPanelLaunch->hide();
      ui.textCoordX->hide();
      ui.textCoordXLabel->hide();
      ui.textCoordY->hide();
      ui.textCoordYLabel->hide();
      ui.textPixelVal->hide();
      ui.textPixelValLabel->hide();
      ui.fpsLabel->hide();
      ui.fpsGage->hide();
      
      
      ui.ageLabel->hide();
      ui.ageGage->hide();

      ui.horizontalSpacer->changeSize(0,0,QSizePolicy::Maximum,QSizePolicy::Maximum);
      ui.horizontalSpacer_2->changeSize(0,0,QSizePolicy::Maximum,QSizePolicy::Maximum);
      ui.horizontalSpacer_3->changeSize(0,0,QSizePolicy::Maximum,QSizePolicy::Maximum);
      ui.horizontalSpacer_4->changeSize(0,0,QSizePolicy::Maximum,QSizePolicy::Maximum);
      ui.horizontalSpacer_5->changeSize(0,0,QSizePolicy::Maximum,QSizePolicy::Maximum);
      
      resize(width(), height()-dsize);
      QIcon icon;
      icon.addFile(QString::fromUtf8(":/icons/arrow_down_128.png"), QSize(), QIcon::Normal, QIcon::Off);
      ui.expandButton->setIcon(icon);
      called = 1;
   }
   else
   {
      ui.buttonReStretch->show();
      ui.buttonFreezeRealTime->show();
      ui.buttonPanelLaunch->show();
      ui.textCoordX->show();
      ui.textCoordXLabel->show();
      ui.textCoordY->show();
      ui.textCoordYLabel->show();
      ui.textPixelVal->show();
      ui.textPixelValLabel->show();
      ui.fpsLabel->show();
      ui.fpsGage->show();
      
      ui.ageLabel->show();
      ui.ageGage->show();

      ui.horizontalSpacer->changeSize(0,20,QSizePolicy::Maximum,QSizePolicy::Minimum);
      ui.horizontalSpacer_2->changeSize(0,20,QSizePolicy::Maximum,QSizePolicy::Minimum);
      ui.horizontalSpacer_3->changeSize(0,20,QSizePolicy::Maximum,QSizePolicy::Minimum);
      ui.horizontalSpacer_4->changeSize(0,20,QSizePolicy::Maximum,QSizePolicy::Minimum);
      ui.horizontalSpacer_5->changeSize(0,20,QSizePolicy::Maximum,QSizePolicy::Minimum);
      resize(width(), height()+dsize);
      QIcon icon;
      icon.addFile(QString::fromUtf8(":/icons/arrow_up_128.png"), QSize(), QIcon::Normal, QIcon::Off);
      ui.expandButton->setIcon(icon);
      called = 0;
   }
}


void imviewerForm::doLaunchStatsBox()
{
   statsBox->setVisible(true);
   
   if(!imStats)
   {
      imStats = new imviewerStats(IMDATA_TYPE,this, 0);
      imStats->setAttribute(Qt::WA_DeleteOnClose); //Qt will delete imstats when it closes.
#if RT_SYSTEM == RT_SYSTEM_VISAO
      if(applyDark) imStats->set_imdata(imdata, frame_time, dark_sim.imdata);
      else imStats->set_imdata(imdata, frame_time, 0);
#else
      imStats->set_imdata(imdata, frame_time, 0);
#endif
      connect(imStats, SIGNAL(finished(int )), this, SLOT(imStatsClosed(int )));
   }

   statsBoxMoved(statsBox->rect());

   imStats->show();
    
   imStats->activateWindow();
}

void imviewerForm::doHideStatsBox()
{
   statsBox->setVisible(false);

   if (imStats)
   {
      //imStats->hide();
      imStats->close(); 
      imStats = 0; //imStats is set to delete on close
   }
}

void imviewerForm::imStatsClosed(int result)
{
   statsBox->setVisible(false);
   imStats = 0; //imStats is set to delete on close
   if(imcp)
   {
      imcp->statsBoxButtonState = false;
      imcp->ui.statsBoxButton->setText("Show Stats Box");
   }
   //imcp->on_statsBoxButton_clicked();
   //doHideStatsBox();
}

void imviewerForm::statsBoxMoved(const QRectF & newr)
{

   QPointF np = qpmi->mapFromItem(statsBox, QPointF(statsBox->rect().x(),statsBox->rect().y()));
   QPointF np2 = qpmi->mapFromItem(statsBox, QPointF(statsBox->rect().x()+statsBox->rect().width(),statsBox->rect().y()+statsBox->rect().height()));
   //std::cout << np.x() << " " << nx - np.y() << " " << np2.x() << " " << nx - np2.y() << "\n\n";

   if(imStats) 
   {
#if RT_SYSTEM == RT_SYSTEM_VISAO
      if(applyDark) imStats->set_imdata(imdata, frame_time, nx, ny, np.x() + .5, np2.x(), ny-np2.y()+.5, ny-np.y(), dark_sim.imdata);
      else imStats->set_imdata(imdata, frame_time, nx, ny, np.x() + .5, np2.x(), ny-np2.y()+.5, ny-np.y(), 0);
#else
      imStats->set_imdata(imdata, frame_time, nx, ny, np.x() + .5, np2.x(), ny-np2.y()+.5, ny-np.y(), 0);
#endif 
   }
   
   //std::cout << np.x() + .5 << " "  <<  np2.x() << " " <<  ny-np2.y()+.5 << " " <<  ny-np.y() << "\n";
}

void imviewerForm::statsBoxRejectMouse()
{
   statsBox->stackBefore(userBox);
   statsBox->stackBefore(guideBox);
   statsBox->stackBefore(userCircle);
}

void imviewerForm::userBoxMoved(const QRectF & newr)
{
   
   QPointF np = qpmi->mapFromItem(userBox, QPointF(newr.x(),newr.y()));
   QPointF np2 = qpmi->mapFromItem(userBox, QPointF(newr.x()+newr.width(),newr.y()+newr.height()));

   userBox_j0 = (int) (np2.x() + .5);
   userBox_j1 = (int) np.x();
   userBox_i0 = ny-(int) (np2.y() + .5);
   userBox_i1 = ny-(int) np.y();

   setUserBoxActive(true); //recalcs and recolors.
   
   
}

void imviewerForm::userBoxRejectMouse()
{
   
   userBox->stackBefore(statsBox);
   userBox->stackBefore(guideBox);
   userBox->stackBefore(userCircle);
   
}


void imviewerForm::guideBoxMoved(const QRectF & newr)
{
   
   QPointF np = qpmi->mapFromItem(guideBox, QPointF(newr.x(),newr.y()));
   QPointF np2 = qpmi->mapFromItem(guideBox, QPointF(newr.x()+newr.width(),newr.y()+newr.height()));

   
   guideBox_i0 = np.x() + .5;
   guideBox_i1 = np2.x();
   
   guideBox_j0 = ny-np2.y() + .5;
   guideBox_j1 = ny-np.y();
  
   //std::cout << guideBox_i0 << " " << guideBox_j0 << " " << guideBox_i1 << " " << guideBox_j1 << "\n";
   
#if RT_SYSTEM == RT_SYSTEM_VISAO
   if(statusboard_shmemptr > 0)
   {
      VisAO::imviewer_status_board * isb = (VisAO::imviewer_status_board *) statusboard_shmemptr;
      
      isb->guidebox_x0 = guideBox_i0;
      isb->guidebox_x1 = guideBox_i1;
      isb->guidebox_y0 = guideBox_j0;
      isb->guidebox_y1 = guideBox_j1;
   }
#endif

}

void imviewerForm::guideBoxRejectMouse()
{
   
   guideBox->stackBefore(statsBox);
   guideBox->stackBefore(userBox);
   guideBox->stackBefore(userCircle);
}

void imviewerForm::userCircleResized(const float &rad)
{
   //std::cout << rad << "\n";

   char tmp[256];
   snprintf(tmp, 256, "%0.1f", rad);
   
   ui.graphicsView->coords->setText(tmp);
}

void imviewerForm::userCircleMoved(const QRectF &newr)
{
   QPointF np = qpmi->mapFromItem(userCircle, QPointF(newr.x(),newr.y()));
   
   float x = np.x()+.5*newr.width();
   float y = np.y()+.5*newr.height();
   
   //std::cout << ScreenZoom << " " << ZoomLevel << "\n";
   ui.graphicsView->coords->setGeometry(x*ScreenZoom-35., y*ScreenZoom-20., 70,40);
  
}

void imviewerForm::userCircleMouseIn()
{
   ui.graphicsView->coords->setVisible(true);
   
}
void imviewerForm::userCircleMouseOut()
{
   ui.graphicsView->coords->setVisible(false);
}

void imviewerForm::userCircleRejectMouse()
{
   
   userCircle->stackBefore(statsBox);
   userCircle->stackBefore(guideBox);
   userCircle->stackBefore(userBox);
   
}


void imviewerForm::post_setUserBoxActive(bool usba)
{
   userBox->setVisible(usba);
}


void imviewerForm::keyPressEvent(QKeyEvent * ke)
{

   if(ke->text() == "p")
   {
      launchControlPanel();
      return;
   }

   if(ke->text() == "r")
   {
      on_buttonReStretch_clicked();
      return;
   }

   if(ke->text() == "s")
   {
      if(statsBox->isVisible())
      {
         doHideStatsBox();
         if(imcp)
         {
            imcp->statsBoxButtonState = false;
            imcp->ui.statsBoxButton->setText("Show Stats Box");
         }
      }
      else
      {
         doLaunchStatsBox();
         if(imcp)
         {
            imcp->statsBoxButtonState = true;
            imcp->ui.statsBoxButton->setText("Hide Stats Box");
         }
      }
      return;
   }

   if(ke->text() == "x")
   {
      on_buttonFreezeRealTime_clicked();
      return;
   }
   
   if(ke->text() == "1")
   {
      set_ZoomLevel(1.0);
      return;
   }
   
   if(ke->text() == "2")
   {
      set_ZoomLevel(2.0);
      return;
   }
   
   if(ke->text() == "3")
   {
      set_ZoomLevel(3.0);
      return;
   }
   
   if(ke->text() == "4")
   {
      set_ZoomLevel(4.0);
      return;
   }
   
   if(ke->text() == "5")
   {
      set_ZoomLevel(5.0);
      return;
   }
   
   if(ke->text() == "6")
   {
      set_ZoomLevel(6.0);
      return;
   }
   
   if(ke->text() == "7")
   {
      set_ZoomLevel(7.0);
      return;
   }
   
   if(ke->text() == "8")
   {
      set_ZoomLevel(8.0);
      return;
   }
   
   if(ke->text() == "9")
   {
      set_ZoomLevel(9.0);
      return;
   }
   
   if(ke->text() == "b")
   {
      if(!userBoxActive)
      {
         if(imcp)
         {
            imcp->on_scaleModeCombo_activated(imviewer::minmaxbox);
         }
         else
         {
            userBox->setVisible(true);
            setUserBoxActive(true);
         }
      }
      else
      {
         if(imcp)
         {
            imcp->on_scaleModeCombo_activated(imviewer::minmaxglobal);
         }
         else
         {
            userBox->setVisible(false);
            setUserBoxActive(false);
         }
      }
      return;
   }
   
   if(ke->text() == "n")
   {
      if(!nup) return;
      if(nup->isVisible())
      {
         nup->setVisible(false);
         nup_tip->setVisible(false);
      }
      else
      {
         nup->setVisible(true);
         nup_tip->setVisible(true);
      }
      return;
   }

   if(ke->text() == "g")
   {
      if(!guideBox->isVisible())
      {
         guideBox->setVisible(true);
         
      }
      else
      {
         guideBox->setVisible(false);
         
      }
      return;
   }
   
   if(ke->text() == "t")
   {
      if(targetVisible)
      {
         cenLineVert->setVisible(false);
         cenLineHorz->setVisible(false);
         targetVisible = false;
      }
      else
      {
         cenLineVert->setVisible(true);
         cenLineHorz->setVisible(true);
         targetVisible=true;
      }
   } 
   
   if(ke->text() == "c")
   {
      set_viewcen(.5, .5);
      post_set_ZoomLevel();
   }
   
   if(ke->text() == "o")
   {
      if(userCircle->isVisible()) userCircle->setVisible(false);
      else userCircle->setVisible(true);
      
   }
   
   QWidget::keyPressEvent(ke);
}
