#include "Converter.h"

//-------------------------------------------------------------------
//-----------------------------Header--------------------------------
void Converter::createHeader() {
  if (BMPtoAB) {
    header.name = 16706; //((Uint16)('A') << 8) + (Uint16)'B'
    if (blacknWhite) {
      header.info = 128;
      if (huffman)
        header.info += 1;
      else if (byterun)
        header.info += 2;
    }
    header.width = img->w;
    header.height = img->h;

    insertHeader();

  } else {
    fileIn >> header.name;
    if (header.name != 16706) {
      std::cerr << "Input file is not .AB" << std::endl;
      exit(1);
    }

    fileIn >> header.info >> header.width >> header.height;
    img = SDL_CreateRGBSurface(0, header.width, header.height, 24, 0, 0, 0, 0);
    if (img == NULL) {
      std::cerr << "SDL_CreateRGBSurface() failed" << std::endl;
      exit(1);
    }
    switch (header.info) {
    case 0:
      blacknWhite = false;
      bitconv = true;
      break;
    case 1:
      blacknWhite = false;
      huffman = true;
      break;
    case 2:
      blacknWhite = false;
      byterun = true;
      break;
    case 128:
      blacknWhite = true;
      bitconv = true;
      break;
    case 129:
      blacknWhite = true;
      huffman = true;
      break;
    case 130:
      blacknWhite = true;
      byterun = true;
      break;
    }
  }
}
//-------------------------------------------------------------------
void Converter::insertHeader() {
  fileOut << header.name << header.info << header.width << header.height;
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void Converter::clear() {
  if (BMPtoAB) {
    fileOut.close();
  } else {
    fileIn.close();
  }

  SDL_FreeSurface(img);
  img = NULL;
}
//-------------------------------------------------------------------
Uint8 *Converter::loadBMPtoTab(SDL_Surface *img) {
  SDL_Color color;
  int numOfPixels = (img->w) * (img->h);
  Uint8 *p = new Uint8[numOfPixels * 3];
  Uint8 *pstart;

  if (p == nullptr) {
    std::cerr << "Error in: loadBMPtoTab: unable to allocate memory for table"
              << std::endl;
    exit(1);
  }
  /* /\
 Nalezy zmienic na try catcha
 w przypadku zlej alokacji jest rzucany wyjatek
*/
  pstart = p;
  for (int y = 0; y < img->h; ++y) {
    for (int x = 0; x < img->w; ++x) {
      SDL_GetRGB(getpixel(img, x, y), img->format, &color.r, &color.g,
                 &color.b);
      *(p++) = color.r / 8;
      *(p++) = color.g / 8;
      *(p++) = color.b / 8;
    }
  }
  return pstart;
}
//-------------------------------------------------------------------
Uint32 Converter::getpixel(SDL_Surface *surface, int x, int y) {
  if ((x >= 0) && (x < surface->w) && (y >= 0) && (y < surface->h)) {
    // SDL_LockSurface sets up a surface for directly accessing the pixels
    SDL_LockSurface(surface);

    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
      return *p;
      break;

    case 2:
      return *(Uint16 *)p;
      break;

    case 3:
      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        return p[0] << 16 | p[1] << 8 | p[2];
      else
        return p[0] | p[1] << 8 | p[2] << 16;
      break;

    case 4:
      return *(Uint32 *)p;
      break;

    default:
      return 0; /* shouldn't happen, but avoids warnings */

      // SDL_UnlockSurface releases the surface
      SDL_UnlockSurface(surface);
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
void Converter::openFile() {
  if (BMPtoAB) {
    img = SDL_LoadBMP(inputFileName.c_str()); // load BMP to SDL_Surface

    fileOut.open(outputFileName.c_str(), std::ios::out | std::ios::binary);
    if (!fileOut) {
      fputs("File error", stderr);
      exit(1);
    }

  } else {
    fileIn.open(inputFileName.c_str(), std::ios::in | std::ios::binary);
    if (!fileIn) {
      fputs("File error", stderr);
      exit(1);
    }
  }
}
//---------------------------------------------------------------------------
void Converter::bit8to5() {

  // SDL_Surface *img = SDL_LoadBMP(inputFile.c_str());
  Uint8 *p = nullptr, *pstart = nullptr;
  int numOfPixels = img->h * img->w;
  int numOfBuffers = ceil((numOfPixels * 3) / 8.); // ilosc buforow 5x8bit
  int lastBufferSize =
      (numOfPixels * 3) % 8; // ilosc wpisanych kolorow do ostatniego bufora

  BMPtoAB = true; // tymczasowo

  p = loadBMPtoTab(img);

  pstart = p;

  for (int i = 0; i < numOfBuffers - 1; ++i) {
    memset(&buffer, 0, sizeof(buffer));

    buffer[0] = (*p << 3) + (*(p + 1) >> 2);
    buffer[1] = (*(p + 1) << 6) + (*(p + 2) << 1) + (*(p + 3) >> 4);
    buffer[2] = (*(p + 3) << 4) + (*(p + 4) >> 1);
    buffer[3] = (*(p + 4) << 7) + (*(p + 5) << 2) + (*(p + 6) >> 3);
    buffer[4] = (*(p + 6) << 5) + *(p + 7);

    fileOut.write(reinterpret_cast<char *>(&buffer), 5);
    p += 8;
  }

  // ostatni bufor
  switch (lastBufferSize) {
  case 0:
    buffer[4] = *(p + 7);
  case 7:
    buffer[4] = buffer[4] + (*(p + 6) << 5);
    buffer[3] = (*(p + 6) >> 3);
  case 6:
    buffer[3] = buffer[3] + (*(p + 5) << 2);
  case 5:
    buffer[3] = buffer[3] + (*(p + 4) << 7);
    buffer[2] = (*(p + 4) >> 1);
  case 4:
    buffer[2] = buffer[2] + (*(p + 3) << 4);
    buffer[1] = (*(p + 3) >> 4);
  case 3:
    buffer[1] = buffer[1] + (*(p + 2) << 1);
  case 2:
    buffer[1] = buffer[1] + (*(p + 1) << 6);
    buffer[0] = (*(p + 1) >> 2);
  case 1:
    buffer[0] = buffer[0] + (*p << 3);
    break;
  default:
    std::cout << "Error";
  }
  fileOut.write(reinterpret_cast<char *>(&buffer), 5);

  delete[] pstart;
}
//---------------------------------------------------------------------------
void Converter::bit5to8() {

  // \/ wartosci height i width zamienic na te z headera
  Uint16 height = 512, width = 512;
  int numOfPixels = height * width;
  int numOfBuffers = ceil((numOfPixels * 3) / 8.); // ilosc buforow 5x8bit
  int lastBufferSize =
      (numOfPixels * 3) % 8; // ilosc wpisanych kolorow do ostatniego bufora
  Uint8 *p = new Uint8[numOfPixels * 3];
  Uint8 *pstart = p;
  SDL_Color color;
  Uint32 pixel;

  BMPtoAB = false; // tymczasowo

  // \/ rezerwowanie miejsca dla SDL_Surface o dopowiednich w i h
  // output = SDL_CreateRGBSurface(0, width, height, 24, 0, 0, 0, 0);
  // if (output == NULL) {
  //  std::cout << "SDL_CreateRGBSurface() failed" << std::endl;
  //  exit(1);
  //}

  for (int i = 0; i < numOfBuffers - 1; ++i) {
    // wczytywanie do bufora

    fileIn.read(reinterpret_cast<char *>(&buffer), 5);

    // przeksztalcenia bitowe oraz zapis do duzej tablicy p
    *(p++) = (buffer[0] >> 3) & 31;
    *(p++) = ((buffer[0] & 7) << 2) + ((buffer[1] >> 6) & 3);
    *(p++) = (buffer[1] >> 1) & 31;
    *(p++) = ((buffer[1] & 1) << 4) + ((buffer[2] >> 4) & 15);
    *(p++) = ((buffer[2] & 15) << 1) + ((buffer[3] >> 7) & 1);
    *(p++) = (buffer[3] >> 2) & 31;
    *(p++) = ((buffer[3] & 3) << 3) + ((buffer[4] >> 5) & 7);
    *(p++) = buffer[4] & 31;
  }

  if (lastBufferSize == 0)

    fileIn.read(reinterpret_cast<char *>(&buffer), 5);
  else
    fileIn.read(reinterpret_cast<char *>(&buffer), lastBufferSize);

  // ostatni bufor, w zaleznosci ile zostalo w nim kolorow
  switch (lastBufferSize) {
  case 0:
    *(p + 7) = buffer[4] & 31;
  case 7:
    *(p + 6) = ((buffer[3] & 3) << 3) + ((buffer[4] >> 5) & 7);
  case 6:
    *(p + 5) = (buffer[3] >> 2) & 31;
  case 5:
    *(p + 4) = ((buffer[2] & 15) << 1) + ((buffer[3] >> 7) & 1);
  case 4:
    *(p + 3) = ((buffer[1] & 1) << 4) + ((buffer[2] >> 4) & 15);
  case 3:
    *(p + 2) = (buffer[1] >> 1) & 31;
  case 2:
    *(p + 1) = ((buffer[0] & 7) << 2) + ((buffer[1] >> 6) & 3);
  case 1:
    *p = (buffer[0] >> 3) & 31;
    break;
  default:
    std::cout << "Error";
  }

  // tworzenie pixeli z tablicy p i zapisywanie do SDL_Surface
  p = pstart;
  SDL_LockSurface(img);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      color.r = *(p++) * 8;
      color.g = *(p++) * 8;
      color.b = *(p++) * 8;
      pixel = SDL_MapRGB(img->format, color.r, color.g, color.b);
      putpixel(img, x, y, pixel);
    }
  }
  SDL_UnlockSurface(img);

  // SDL_SaveBMP
  SDL_SaveBMP(img, outputFileName.c_str());

  delete[] pstart;
}
//---------------------------------------------------------------------------
void Converter::putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch (bpp) {
  case 1:
    *p = pixel;
    break;

  case 2:
    *(Uint16 *)p = pixel;
    break;

  case 3:
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
      p[0] = (pixel >> 16) & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = pixel & 0xff;
    } else {
      p[0] = pixel & 0xff;
      p[1] = (pixel >> 8) & 0xff;
      p[2] = (pixel >> 16) & 0xff;
    }
    break;

  case 4:
    *(Uint32 *)p = pixel;
    break;
  }
}
//---------------------------------------------------------------------------
void Converter::ByteRun() {
  if (BMPtoAB)
    if (blacknWhite)
      ByteRunCoderBlackNWhite();
    else
      ByteRunCoderRGB();
  else
    ByteRunDecoder();
}
void Converter::ByteRunCoderRGB() {


  // posiadam SDL_Surface *img -> załadowana bitmapa z pixelami
  // img->w = width
  // img->h = height
  // lub mogę loadBMPtoTAB i wtedy mam tablice od 0 do n w Uint8
  // ale to zajmuje dużo RAMu, ale nei trzeba obliczać indexów i
  // zaoszczędzimy czas na funkcji getpixel
  // pod fileIn mam uchwyt do pliku wejscie
  // pod fileOut gotowy zapsis uchwyt z wklejonym headeerm


  // tworzymy tablice 3*w*h
  // wpisujemy do niej kolory najpeirw całe R, potem G potem B
  // odpalamy ByteRun na tej tablicy i do pliku.
  // dzieki tm trzem kanalom mamy wieksze prawdopodobienstwo na udana kompresje

  //=---=--=-=-=--= Programix

  // Ładowanie BMP do tab (najpeirw same R, potem same G, potem same B);

  SDL_Color color;
  int numOfPixels = (img->w) * (img->h);
  Uint8 *tab = new Uint8[numOfPixels * 3];


  if (tab == nullptr) {
    std::cerr << "Error in: loadBMPtoTab: unable to allocate memory for table"
              << std::endl;
    exit(1);
  }
  /* /\
 Nalezy zmienic na try catcha
 w przypadku zlej alokacji jest rzucany wyjatek
*/
//  Uint8 *pstart = tab;
  Uint8 *kanalR = tab;
  Uint8 *kanalG = tab + numOfPixels;
  Uint8 *kanalB = tab + ( 2 * numOfPixels);

  for (int y = 0; y < img->h; ++y) {
    for (int x = 0; x < img->w; ++x) {
      SDL_GetRGB(getpixel(img, x, y), img->format, &color.r, &color.g,
                 &color.b);
      *(kanalR++) = color.r / 8;
      *(kanalG++) = color.g / 8;
      *(kanalB++) = color.b / 8;
    }
  }



  // ByteRun dla RGB // w sumei tez dla BlackNWhite, tylko podamy inna tablice;
  int i = 0;
  int size = 3 * numOfPixels;
  // wykonujemy do konca talbicy
  while (i < size){
  //  jezeli nie jestesmy na ostatnim elemencie i element kolejny jest taki sam
  //  to mierzymy dlugosc sekwencji
    if ((i < size-1) && (tab[i] == tab[i+1])){
  // pomiar dlugosci sekwencji
      int len = 0;
      while ((i+len < size-1) && (tab[i+len] == tab[i+len+1]) && (len <= 126)){ // maksymalna dlugosc sekwencji = 126
        len++;
      }
      //wypisz do pliku
      fileOut << (Uint8)-len << "," << tab[i+len] << ",";
      //przesun wskaznik o dlugosc sekwencji
      i += (len+1);
    }
    //sekwencja roznych wartosci
    else{
      //zmierz dlugosc sekwencji
      // maksymalna dlugosc = 127
      int len=0;
      while ((i+len < size-1) && (tab[i+len] != tab[len+i+1]) && (len <= 127)){
        len++;
      }
      //dodajemy ostatni bajt, jezeli jest taki sam, w celu lepszej kompresji
      if ((i+len == size-1) && (len < 128)){
        len++;
      }
      //wypisz spakowana sekwencje
      fileOut << (len-1) << ",";
      for (int j=0; j<len; j++){
        fileOut << (Uint8)tab[i+j] << ",";
      }
      //przesun wskaznik o dlugosc sekwencji
      i += len;
    }
  }







}
void Converter::ByteRunCoderBlackNWhite() {


  //int x = 0;

 }
void Converter::ByteRunDecoder(){
  // RGB
  // mamy plik zakodowany full najpierw R potem G potem B
  // tworzymy nowy plik i dekodujemy ten do kolejnego
  // wczytujemy po kolei najpeirw piksele R, potem G, potem B
  // czyli 3 petle for dla kazdego kanalu




};







// end
