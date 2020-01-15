#include "scene3d.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime())); //Случайное число зависит от времени

    a.addLibraryPath(a.applicationDirPath()+"/platforms");

    scene3D w;
    w.resize(600, 600);
    w.show();

    return a.exec();
}
