#include "mainwindow.h"
#include <QApplication>
#include <SDL/SDL.h>

using namespace std;
int main(int argc, char *argv[])
{

  // console output
  freopen("CON", "wt", stdout);
  freopen("CON", "wt", stderr);


    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
