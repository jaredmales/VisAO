

#include "ui_BasicBCU39Form.h"

#include "VisAOApp_base.h"
#include "libvisao.h"

#include <QWidget>
#include <sstream>
#include <QMessageBox>
#include <QFileDialog>

#include "../basic_ui.h"
#include "rtplot.h"

namespace VisAO
{
   
class BasicBCU39Form : public VisAO::basic_ui
{
   Q_OBJECT
   
public:
   BasicBCU39Form(QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicBCU39Form(int argc, char **argv, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   BasicBCU39Form(std::string name, const std::string& conffile, QWidget * Parent = 0, Qt::WindowFlags f = 0);
   

   void Create();

protected:
   
   void attach_basic_ui();
   
   double wait_to;

   void retrieve_state();
   
   void update_status();

   int fg39stat;
   int saving;
   int skipping;
   int remaining;
   int reconstat;

   rtPlotForm * strehlPlot;

   rtPlotForm * wfePlot;
   rtPlotForm * ttPlot;
   rtPlotForm * ho1Plot;
   rtPlotForm * ho2Plot;
   
   sharedim_stack<float> strehl_sis; ///<Shared memory ring buffer for strehl data
   sharedim<float> strehl_sim; ///<Shared memory image, from ring buffer, for strehl data
   bool strehlAttached;
   int curr_strehl, last_strehl;

   QTimer strehlPlotTimer; ///< When this times out update the Strehl plot.
   int strehlPlotUpdateInt; ///<Interval at which to update the Strehl plot.

   framegrabber39_status_board * fg39sb;
   basic_status_board * fw39sb;
   reconstructor_status_board * rsb;
   
   void updateFW39Status();

   void updateRecStatus();
   
   double lambda;
   double cal_a;
   double cal_b;
   
   bool applyFitError;

   bool plotRawStrehl;
   bool plotFilteredStrehl;
   bool plotRMS;

   std::string fullRecPath;
   QString recPath;
   std::string fullFilterPath;
   QString filterPath;
   
public:
   void setLambda(double);
   
protected slots:
   
    /** @name Framegrabber controls
     */ 
   //@{
   
   void on_startButton_clicked();
   
   //@}
   
   /** @name Framewriter controls
     */ 
   //@{ 
   
   void on_saveDirButton_clicked();
   void on_saveCont_stateChanged(int);
   void on_saveButton_clicked();
   
   //@}
   
   /** @name Reconstructor controls
    */ 
   //@{ 
   
   void on_recFileButton_clicked();
   void on_lambdaEdit_editingFinished();
   void on_lambdaBox_activated(int);
   void on_filterFileButton_clicked();
   void on_calA_editingFinished();
   void on_calB_editingFinished();
   void on_reconButton_clicked();

   //@}
   
 
   void on_fitCheck_stateChanged(int);
   void on_rawCheck_stateChanged(int);
   void on_filteredCheck_stateChanged(int);
   void on_rmsCheck_stateChanged(int);
   
   void on_ButtonTakeLocal_clicked();
   
   void on_plotButton_clicked();
   void on_plotWFEButton_clicked();
   void on_plotTTWFEButton_clicked();
   void on_plotHO1WFEButton_clicked();
   void on_plotHO2WFEButton_clicked();
   
   void updateStrehlPlot();
   
private:
   
   Ui::BasicBCU39Form ui;
   
};

} //namespace VisAO
