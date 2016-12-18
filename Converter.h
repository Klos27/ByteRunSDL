#include <SDL/SDL.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

class Converter {

  // header structure
  struct header{
    Uint16 name;
    Uint8 info;
    Uint16 width;
    Uint16 height;

    header():info(0){}
  } header;

  std::fstream fileIn, fileOut; // to read/write from/to files
  unsigned char buffer[5];
  Uint8 *loadBMPtoTab(SDL_Surface *img); // loading every pixels RGB of BMP to
                                         // table and returning pointer
  SDL_Surface *img = NULL;

  Uint32 getpixel(SDL_Surface *surface, int x, int y);
  void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
  void insertHeader();

public:
  // *********************************
  void createHeader();
  void openFile();
  void clear();

  // Tutaj maja byc wasze funkcje
  void bit8to5();
  void bit5to8(); // 5bit->8bit to .bmp
  void ByteRun(); // Byte Run for 8bit


  //****
  bool blacknWhite = false; // if true image is black and white //uwzglednic to w funkcjach konwertujacych
  bool BMPtoAB = false;     // if false AB->BMP else BMP->AB
  std::string inputFileName;
  std::string outputFileName;
  // Algorithms
  // *************************
  bool bitconv = false;
  bool huffman = false;
  bool byterun = false;
  // *************************
  //****
private:
  void ByteRunCoder();
  void ByteRunDecoder();


};
