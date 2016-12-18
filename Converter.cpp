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
    printf("coder: width: %d , height: %d\n", header.width, header.height);
  //insertHeader();

  } else {
    /*fileIn >> header.name;
    if (header.name != 16706) {
      std::cerr << "Input file is not .AB" << std::endl;
      exit(1);
    }

    fileIn >> header.info >> header.width >> header.height;
*/

    ///////------------------ tymczasowo bo cos sie sypie
    header.width = 1920;
    header.height = 1080;
    /////////////----------------
    img = SDL_CreateRGBSurface(0, header.width, header.height, 24, 0,0,0, 0);
    if (img == NULL) {
      std::cerr << "SDL_CreateRGBSurface() failed" << std::endl;
      exit(1);
    }
    printf("decoder: width: %d , height: %d\n", header.width, header.height);
  /* switch (header.info) {
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
    }*/
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
    ByteRunCoder();
  else
    ByteRunDecoder();
}
void Converter::ByteRunCoder() {
  SDL_Color color;
  int numOfPixels = (img->w) * (img->h);
  int size; // rozmiar tablicy

  if(blacknWhite)
    size = numOfPixels;
  else
    size = numOfPixels * 3;

  Uint8 *tab = new Uint8[size]; // tablica z pixelami
  if (tab == nullptr) {
    std::cerr << "Error in: loadBMPtoTab: unable to allocate memory for table" << std::endl;
    exit(1);
  }
  // Ładowanie BMP do tab (srednia z 3 kolorow -> Black & White)
  if(blacknWhite){
    Uint8 *p = tab;
    for (int y = 0; y < img->h; ++y) {
      for (int x = 0; x < img->w; ++x) {
        SDL_GetRGB(getpixel(img, x, y), img->format, &color.r, &color.g, &color.b);
        *p = ((color.r+color.g+color.b)/3);
        p++;
      }
    }
  }
  else {
    // Ładowanie BMP do tab (najpeirw same R, potem same G, potem same B);
    Uint8 *kanalR = tab;
    Uint8 *kanalG = tab + numOfPixels;
    Uint8 *kanalB = tab + ( 2 * numOfPixels);

    for (int y = 0; y < img->h; ++y) {
      for (int x = 0; x < img->w; ++x) {
        SDL_GetRGB(getpixel(img, x, y), img->format, &color.r, &color.g, &color.b);
        *kanalR = color.r;
        *kanalG = color.g;
        *kanalB = color.b;
        kanalR++;kanalG++;kanalB++;
      }
    }
  }

  int i = 0;
  signed char wypisz = 0;
  // ByteRun po kanałach R,G,B
  while (i < size){
    //  jezeli nie jestesmy na ostatnim elemencie i element kolejny jest taki sam
    //  to mierzymy dlugosc sekwencji
    if ((i < size-1) && (tab[i] == tab[i+1])){
      int len = 1;
      // maksymalna dlugosc sekwencji -> 128
      while ((i+len < size-1) && (tab[i+len] == tab[i+len+1]) && (len < 127)){
        len++;
      }

      wypisz = (char)-len;
      fileOut.write(reinterpret_cast<char *>(&wypisz), 1);
      fileOut.write(reinterpret_cast<char *>(&tab[i]), 1);
      //przesun wskaznik
      i += len+1;
    }
    //sekwencja roznych wartosci
    else {
      // maksymalna dlugosc -> 128
      int len=1;
      while ((i+len < size-1) && (tab[i+len] != tab[len+i+1]) && (len < 128)){
        len++;
      }
      //dodajemy ostatni bajt, jezeli jest taki sam, w celu lepszej kompresji
      if ((i+len == size-1) && (len < 128)){
        len++;
      }
      len--;
      wypisz = (char)len;
      fileOut.write(reinterpret_cast<char *>(&wypisz), 1);
      for (int j=0; j<len+1; j++){
        // tab[i] bo i sie zmienia z kazdym przejsciem petli
        fileOut.write(reinterpret_cast<char *>(&tab[i]), 1);
        i++;
      }
    }
  }
}

void Converter::ByteRunDecoder(){

  int numOfPixels = (img->w) * (img->h);
  int size; // rozmiar tablicy

  if(blacknWhite)
    size = numOfPixels;
  else
    size = numOfPixels *3;

  int i = 0;
  Uint8 p;

  Uint8 *tab = new Uint8[size]; // tablica z pixelami
  if (tab == nullptr) {
    std::cerr << "Error in: loadBMPtoTab: unable to allocate memory for table" << std::endl;
    exit(1);
  }

//-------------- Dekompresja
  signed char len;

  //Dekompresja Byterun do tablicy RGB lub Black&White
  while (i < size){
      fileIn.read(reinterpret_cast<char *>(&len), 1);
      // Kod pusty, nie powinno sie zdarzyc
      if (len == -128){
        i++;
      }
      // Powtorzenie
      else if (len < 0){
        fileIn.read(reinterpret_cast<char *>(&p), 1);
        for (int j=0; j< ((int)-(len))+1; j++){
           tab[i] = p;
           i++;
        }
      }
      //  Kopiowanie (len>=0)
      else {
        for (int j=0; j<((int)(len))+1; j++){
          fileIn.read(reinterpret_cast<char *>(&p), 1);
          tab[i] = p;
          i++;
        }
      }
  }

//--------------------------------------------------------------
    // Zapisywanie do SDL_Surface
    SDL_LockSurface(img);
    SDL_Color color;
    Uint32 pixel;
    int height = img->h, width = img->w;

    if(blacknWhite){
      Uint8 *pstart = tab;
      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          color.r = *pstart;
          color.g = *pstart;
          color.b = *pstart;
          pstart++;
          pixel = SDL_MapRGB(img->format, color.r, color.g, color.b);
          putpixel(img, x, y, pixel);
        }
      }
    }
    else {
      Uint8 *kanalR = tab;
      Uint8 *kanalG = tab + numOfPixels;
      Uint8 *kanalB = tab + ( 2 * numOfPixels);
      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          color.r = *kanalR; kanalR++;
          color.g = *kanalG; kanalG++;
          color.b = *kanalB; kanalB++;
          pixel = SDL_MapRGB(img->format, color.r, color.g, color.b);
          putpixel(img, x, y, pixel);
        }
      }
    }
    SDL_UnlockSurface(img);

    // SDL_SaveBMP
    SDL_SaveBMP(img, outputFileName.c_str());

}

/* usprawnienie tego cpy:
int i = 0;
char wypisz = 0;
// ByteRun
while (i < size){
//  jezeli nie jestesmy na ostatnim elemencie i element kolejny jest taki sam
//  to mierzymy dlugosc sekwencji
  if ((i < size-1) && (tab[i] == tab[i+1])){
    // pomiar dlugosci sekwencji
    int len = 0;
    // maksymalna dlugosc sekwencji = 127
    while ((i+len < size-1) && (tab[i+len] == tab[i+len+1]) && (len < 127)){
      len++;
    }

    wypisz = (char)-len;
    fileOut.write(reinterpret_cast<char *>(&wypisz), 1);
    fileOut.write(reinterpret_cast<char *>(&tab[i]), 1);
    //przesun wskaznik o dlugosc sekwencji
    i += len+1;
  }
  //sekwencja roznych wartosci
  else{
    //zmierz dlugosc sekwencji
    // maksymalna dlugosc = 128
    int len=0;
    while ((i+len < size-1) && (tab[i+len] != tab[len+i+1]) && (len < 128)){
      len++;
    }
  //  std::cout <<"przepisz i= " << i  << "len = " << len << std::endl;
    //dodajemy ostatni bajt, jezeli jest taki sam, w celu lepszej kompresji
    if ((i+len == size-1) && (len < 128)){
      len++;
    }
    len--;
    wypisz = (char)len;
    fileOut.write(reinterpret_cast<char *>(&wypisz), 1);
    for (int j=0; j<len+1; j++){
      //fileOut.write(reinterpret_cast<char *>(&tab[i+j]), 1);
      // tab[i] bo i sie zmienia z kazdym przejsciem petli
      fileOut.write(reinterpret_cast<char *>(&tab[i]), 1);

      i++;
    }
  }
}






}

void Converter::ByteRunDecoder(){
int numOfPixels = (img->w) * (img->h);
int size;

if(blacknWhite)
  size = numOfPixels;
else
  size = numOfPixels *3;

int i = 0;
Uint8 p;

Uint8 *tab = new Uint8[size]; // tablica z pixelami
if (tab == nullptr) {
  std::cerr << "Error in: loadBMPtoTab: unable to allocate memory for table"
            << std::endl;
  exit(1);
}

//--------------------------------------------------------------
signed char len;

//dopoki wszystkie bajty nie sa zdekompresowane
while (i < size){
    fileIn.read(reinterpret_cast<char *>(&len), 1);
    //kod pusty
    if (len == -128){
      i++;
    }
    //sekwencja powtarzajacych sie bajtow
    else if (len < 0){
      fileIn.read(reinterpret_cast<char *>(&p), 1);
      for (int j=0; j< ((int)-(len))+1; j++){
         tab[i] = p;
         i++;
      }
    }
    //sekwencja roznych bajtow
    else{
      // kopiowanie bajtow
      for (int j=0; j<((int)(len))+1; j++){
        fileIn.read(reinterpret_cast<char *>(&p), 1);
        tab[i] = p;
        i++;
      }
    }
}

//--------------------------------------------------------------
printf("skonczylem plik tmp; i = %d\n", i);
i=0;
  // zapisywanie do SDL_Surface
  SDL_LockSurface(img);
  SDL_Color color;
  Uint32 pixel;
  int height = img->h, width = img->w;

if(blacknWhite){
  Uint8 *pstart = tab;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      color.r = *pstart;
      color.g = *pstart;
      color.b = *pstart;
      // color.r = *(start++);
      // color.g = *(start++);
      // color.b = *(start++);
      pstart++;
      i++;


      //kanalR++;kanalG++;kanalB++;
      pixel = SDL_MapRGB(img->format, color.r, color.g, color.b);
      putpixel(img, x, y, pixel);
      i++;
    }
  }




} else {
  Uint8 *kanalR = tab;
  Uint8 *kanalG = tab + numOfPixels;
  Uint8 *kanalB = tab + ( 2 * numOfPixels);
//  Uint8 *start = tab;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      color.r = *kanalR;
      color.g = *kanalG;
      color.b = *kanalB;
      i++;


      kanalR++;kanalG++;kanalB++;
      pixel = SDL_MapRGB(img->format, color.r, color.g, color.b);
      putpixel(img, x, y, pixel);
    }
  }
}
  printf("skonczylem SDLSurface; i = %d\n", i);
  SDL_UnlockSurface(img);

  // SDL_SaveBMP
  SDL_SaveBMP(img, outputFileName.c_str());

}

*/











// end



// while (i < size){
//     fileIn.read(reinterpret_cast<char *>(&tab[i]), 1);
//     i++;
// }


// dziala
// while (i < size){
//     fileOut.write(reinterpret_cast<char *>(&tab[i]), 1);
//     i++;
// }
