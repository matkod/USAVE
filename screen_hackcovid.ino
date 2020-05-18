#include <SPI.h>
#include <MCUFRIEND_kbv.h>


// These are 'flexible' lines that can be changed
#define TFT_CS 53
#define TFT_DC 48
#define TFT_RST 8 // RST can be set to -1 if you tie it to Arduino's reset
#define SD_CCS 42


#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define PINK      0xF81F
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49



#define BUFFER_SIZE 50


MCUFRIEND_kbv tft = MCUFRIEND_kbv(TFT_CS, TFT_DC);

boolean display1 = true;
double ox, oy;

double curr_x = 0;  // apenas para gerar o sinal de teste

int counter = 0;
double xx[BUFFER_SIZE], yy[BUFFER_SIZE];


void setup() {
  Serial.begin(115200);

  setup_tft();
}


void loop() {

  // passar os pontos x e y
  draw_graph_buffer(curr_x, sin(curr_x), -1, 1);
  
  curr_x += 0.1;
  
  delay(10);
}


// inicializar display
void setup_tft() {
  Serial.println(F("TFT LCD debug"));

#ifdef USE_ADAFRUIT_SHIELD_PINOUT
  Serial.println(F("Using Adafruit 2.4\" TFT Arduino Shield Pinout"));
#else
  Serial.println(F("Using Adafruit 2.4\" TFT Breakout Board Pinout"));
#endif

  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

  tft.reset();

  uint16_t identifier = tft.readID();
  if (identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if (identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if (identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  } else if (identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if (identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if (identifier == 0x7783) {
    Serial.println(F("Found ST7781 LCD driver"));
  } else if (identifier == 0x8230) {
    Serial.println(F("Found UC8230 LCD driver"));
  }
  else if (identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if (identifier == 0x0101)
  {
    identifier = 0x9341;
    Serial.println(F("Found 0x9341 LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    identifier = 0x9341;

  }

  tft.begin(identifier);

  tft.fillScreen(BLACK);

  tft.setRotation(1);
}


/*
 * Recebe os pontos x e y e quando recebeu 50 desenha o grafico
 * x = valor de x
 * y = valor de y
 * ylo = menor valor de y
 * yhi = maior valor de y
 */
void draw_graph_buffer(double x, double y, double ylo, double yhi) {
  xx[counter] = x;
  yy[counter] = y;

  counter++;

  if (counter == BUFFER_SIZE) {
    counter = 0;
    curr_x = 0;

    unsigned long startMillis = millis();

    display1 = true;
    draw_graph2(tft, xx, yy, ylo, yhi, "U-SAVE", display1);
    draw_info2(tft, 1, "cmH20", 22, "cmH20", 333, "rpm", 255, 35); // mudar os valores

    unsigned long lastMillis = millis();
    Serial.println(lastMillis - startMillis);
  }
}


void draw_graph2(MCUFRIEND_kbv &d, double x[], double y[], double ylo, double yhi, String title, bool &reset_plot) {
  double yinc = (abs(ylo) + abs(yhi)) / 5;

  tft.fillScreen(BLACK);
  //tft.fillRect(0, 40, 320, 200, BLACK);

  for (int k = 0; k < BUFFER_SIZE; ++k) {
    Graph(tft, x[k], y[k], 40, 220, 200, 180, 0, 5, 1, ylo, yhi, .25, title, "x", "y", DKBLUE, RED, YELLOW, WHITE, BLACK, reset_plot);
  }
}


void draw_info2(MCUFRIEND_kbv &d, double value1, String text1, double value2, String text2, double value3, String text3, int posx, int posy) {
  int spacing = 70;
  int value_size = 3;
  int text_size = 2;

  d.setTextColor(YELLOW, BLACK);
  d.setTextSize(value_size);
  d.setCursor(posx, posy);
  d.println(Format(value1, 3, 0));

  d.setTextColor(YELLOW, BLACK);
  d.setTextSize(value_size);
  d.setCursor(posx, posy + spacing * 1);
  d.println(Format(value2, 3, 0));

  d.setTextColor(YELLOW, BLACK);
  d.setTextSize(value_size);
  d.setCursor(posx, posy + spacing * 2);
  d.println(Format(value3, 3, 0));


  d.setTextColor(WHITE, BLACK);
  d.setTextSize(text_size);
  d.setCursor(posx, posy + 30);
  d.println(text1);

  d.setTextColor(WHITE, BLACK);
  d.setTextSize(text_size);
  d.setCursor(posx, posy + 30 + spacing * 1);
  d.println(text2);

  d.setTextColor(WHITE, BLACK);
  d.setTextSize(text_size);
  d.setCursor(posx, posy + 30 + spacing * 2);
  d.println(text3);

}


void Graph(MCUFRIEND_kbv &d, double x, double y, double gx, double gy, double w, double h, double xlo, double xhi, double xinc, double ylo, double yhi, double yinc, String title, String xlabel, String ylabel, unsigned int gcolor, unsigned int acolor, unsigned int pcolor, unsigned int tcolor, unsigned int bcolor, boolean &redraw) {

  double ydiv, xdiv;
  double i;
  double temp;
  int rot, newrot;

  if (redraw == true) {
    redraw = false;
    ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
    oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

    // Desenhar a escala Y
    for ( i = ylo; i <= yhi; i += yinc) {
      // Calcular as transformadas
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

      if (i == 0) {
        d.drawFastHLine(gx, temp, w, acolor);
      }
      else {
        d.drawFastHLine(gx, temp, w, gcolor);
      }

      d.setTextSize(1);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(gx - 40, temp);
      d.println(i);
    }

    // Desenhar a escala X
    for (i = xlo; i <= xhi; i += xinc) {
      // Calcular as transformadas
      temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
      if (i == 0) {
        d.drawFastVLine(temp, gy - h, h, acolor);
      }
      else {
        d.drawFastVLine(temp, gy - h, h, gcolor);
      }
    }

    // Desenhar as labels
    d.setTextSize(2);
    d.setTextColor(tcolor, bcolor);
    d.setCursor(gx , gy - h - 30);
    d.println(title);
  }

  // Desenhar o ponto no grafico
  x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  d.drawLine(ox, oy, x, y, pcolor);
  //d.drawLine(ox, oy + 1, x, y + 1, pcolor);
  //d.drawLine(ox, oy - 1, x, y - 1, pcolor);
  ox = x;
  oy = y;

}


String Format(double val, int dec, int dig) {
  int addpad = 0;
  char sbuf[20];
  String condata = (dtostrf(val, dec, dig, sbuf));


  int slen = condata.length();
  for ( addpad = 1; addpad <= dec + dig - slen; addpad++) {
    condata = " " + condata;
  }
  return (condata);

}
