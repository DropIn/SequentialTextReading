#include "ui_control.h"
#include <qapplication.h>

class ViewerInterface : public QDialog, public Ui::Dialog
{
public:
    ViewerInterface() { setupUi(this); }
	
};

int main_STR(int argc, char** argv);

int main(int argc, char** argv)
{
    
	// Read command lines arguments.
	QApplication application(argc,argv);
	
	// Instantiate the viewer.
	ViewerInterface viewer;
	
#if QT_VERSION < 0x040000
	// Set the viewer as the application main widget.
	application.setMainWidget(&viewer);
#else
	viewer.setWindowTitle("Text Reader Control");
#endif
	
	// Make the viewer window visible on screen.
	viewer.show();
	
	// Run main loop.
	return application.exec();
    
    
//    main_STR(argc, argv);
}
