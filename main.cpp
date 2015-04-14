#include "zoomcameratestwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ZoomCameraTestWidget w;
    w.show();

    return a.exec();
}
