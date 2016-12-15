#include <SDL/SDL.h>
#include "Converter.h"
using namespace std;
int main(int argc, char *argv[])
{
  Converter klasa;

  klasa.byterun = true;
 klasa.BMPtoAB = false;
 //klasa.BMPtoAB = true;
  klasa.blacknWhite = false;

  if(klasa.BMPtoAB == true){
    klasa.inputFileName = "img4.bmp";
    klasa.outputFileName = "output.ab";
  } else {
    klasa.inputFileName = "output.ab";
    klasa.outputFileName = "output.bmp";
  }

  klasa.openFile();
  klasa.createHeader();
  klasa.ByteRun();

  klasa.clear();
  // fileOut.close();
  // fileIn.close();
  // SDL_FreeSurface(klasa.img);
  // klasa.img = NULL;

    return 0;
}
