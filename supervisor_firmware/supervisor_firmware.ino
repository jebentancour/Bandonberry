/*  
Lista de errores
1) Error1: "BRasp_ON"  No se obtuvo la señal de boot OK de la Raspberry al Arduino. Se apagará todo el sistema segundos despues de mostrar el mensaje de error
2) Error2: "BRasp_Off"  No se obtuvo la señal de finializacion del boot OK de la Raspberry al Arduino. Se apagará todo el sistema segundos despues de mostrar el mensaje de error
3) Error3: Por algún motivo la señal de apagado no funciona, o bien no se envía correctamente, o bien el circuito de desconexión tiene algún problema
4) Error4: Error desconocido
5) Error5: No se puede realizar el apagado, puede ser que el uruario no suelta el boton de encendido
*/

#include <SPI.h>
#include <Wire.h>

// Display includes
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Fuel Gauge includes
#include <MAX17043.h>

// Display functtions and constants
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Fuel Gauge functtions and constants
MAX17043 batteryMonitor;

// Boton set pin numbers
const int ShutDownBot_IN  = 6;        // Señal que recibe del botón de encendido/apagado
const int RaspState_IN    = 7;        // Señal de la raspberry que indica High = No apagar, Low = Se puede apagar
const int PowerOnOff_OUT  = 8;        // Señal que enciende el transistor, permitiendo alimentar todo el sistema. High=encendido todo el sistema, Low=Se corta la corriente de todo el sistema (A no ser que se apreiete el botón de encendido) 
const int RaspOff_OUT     = 9;        // Señal que indica a la raspberry que comience el proceso de preparación de apagado. (High 0.5 segundos es un reset, por más de 2  sec es comienzo de proceso de apagado, y por más de 8 sec es apagado forzado)
const int ROn_OUT         = 16;       // Señal que alimenta a la raspberry estando en high, reaultado de chequear la bateria previamente

// Constantes funcionales
const long Time_Out_Boot_Raspb = 120000; // Tiempo maximo de espera del arduino a que la raspberry bootee en ms
const float Umbral_Low_Bat     = 3.4;    // Umbral de voltaje en el cual se acepta encender la Raspberry pero se exige caragador
const float Umbral_Dead_Bat    = 3.0;    // Umbral donde se apaga el sistema

// Definicion de variables
int BState       = LOW;          // Variable de lectura del boton de apagado, LOW es boton presionado
int RState       = LOW;          // Variable de lectura del la estado de la raspberry, HIGH es raspberry NO lista para apagar
float StartTime  = 0;            // Variables para contar el tiempo que esta apretado el boton  
float ResultTime = 0;
int error        = 0;

// Function declaration
void CheckDivices();
void ApagadoNormal();
void FatalErrorList();

/////////////////////////////////////// IMAGENES PARA ANIMACIONES  /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const unsigned char PROGMEM bandoneon1[] =
{0x00,0x00,0x07,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x01,0x72,0x97,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00
,0x00,0x1c,0x88,0x49,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x61,0x0f,0xc0,0x00
,0x01,0xe4,0x45,0x20,0xaf,0x03,0xf8,0x00,0x00,0x0c,0x07,0xc8,0x10,0x06,0x38,0x00
,0x0c,0x22,0x22,0x84,0x04,0xa0,0x23,0xc0,0x00,0xe4,0x22,0x41,0x10,0x86,0x24,0x00
,0x38,0x11,0x00,0x52,0x50,0x91,0x02,0x1c,0x0e,0x42,0x00,0x24,0x88,0x43,0x10,0x00
,0x44,0x08,0x81,0xa9,0x2a,0x40,0x10,0x22,0xe2,0x02,0x11,0x20,0x00,0x01,0x02,0x00
,0x42,0x04,0x40,0x94,0x15,0x08,0x81,0x00,0x00,0x20,0x01,0x02,0x44,0x21,0x89,0x00
,0x41,0x01,0x24,0x45,0x14,0xa4,0x48,0x11,0x11,0x11,0x08,0x11,0x02,0x01,0x84,0x00
,0x60,0x80,0x92,0x2d,0x14,0x90,0x00,0x88,0x80,0x88,0x00,0x81,0x20,0x10,0xc4,0x80
,0x50,0x40,0x49,0x2d,0x14,0x02,0x04,0x00,0x08,0x00,0x88,0x08,0x91,0x08,0xc2,0x00
,0x08,0x20,0x34,0x2d,0x14,0x00,0x40,0x04,0x48,0x40,0x88,0x80,0x90,0x08,0xfe,0x40
,0x04,0x10,0x10,0x2d,0x00,0x00,0x42,0x22,0x48,0x08,0x00,0x00,0x91,0x08,0x43,0x40
,0x3e,0x08,0xd0,0x2d,0x01,0x24,0x02,0x00,0x48,0x08,0x00,0x08,0x91,0x00,0x42,0x40
,0x1d,0x07,0x10,0x2d,0x01,0x24,0x00,0x00,0x40,0x81,0x11,0x08,0x91,0x10,0x81,0xc0
,0x38,0xe2,0x00,0x2d,0x01,0x24,0x00,0x00,0x40,0x81,0x11,0x08,0x11,0x10,0x85,0x80
,0x38,0x02,0x00,0x2d,0x01,0x24,0x84,0x00,0x40,0x91,0x11,0x00,0x10,0x10,0x07,0x40
,0x32,0x32,0x02,0x2d,0x29,0x00,0x84,0x00,0x40,0x10,0x00,0x00,0x10,0x01,0x0f,0x40
,0x12,0x5e,0x02,0x25,0x28,0x00,0x80,0x40,0x11,0x10,0x20,0x10,0x12,0x01,0x0f,0x80
,0x14,0x7e,0x02,0xa5,0x28,0x08,0x80,0x40,0x11,0x02,0x20,0x10,0x12,0x22,0x0f,0x00
,0x1c,0x7e,0x0a,0xa5,0x28,0x48,0x08,0x44,0x11,0x02,0x22,0x10,0x12,0x22,0x1e,0x00
,0x1d,0x7f,0x0a,0xa5,0x22,0x48,0x08,0x44,0x10,0x02,0x02,0x00,0x12,0x00,0x1c,0x00
,0x15,0x3f,0x0a,0xa5,0x22,0x48,0x08,0x44,0x00,0x20,0x02,0x00,0x10,0x04,0x3c,0x00
,0x24,0x3f,0x08,0xa4,0x22,0x41,0x00,0x44,0x02,0x20,0x40,0x20,0x10,0x44,0x38,0x00
,0x2e,0x3f,0x08,0xa4,0x02,0x01,0x00,0x44,0x82,0x24,0x40,0x20,0x14,0x48,0x78,0x00
,0x1e,0xbf,0x00,0xa7,0xd2,0x01,0x10,0x05,0xa0,0x04,0x04,0x20,0x14,0x08,0x70,0x00
,0x0e,0xbf,0x01,0xb0,0x10,0x11,0x13,0xf0,0x3e,0x04,0x04,0x00,0x14,0x00,0x30,0x00
,0x07,0x9f,0x07,0x00,0x06,0x97,0xe0,0x00,0x00,0x7c,0x84,0x01,0xff,0xf8,0xa0,0x00
,0x00,0x5f,0x88,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0xf4,0x5c,0x00,0x01,0xc0,0x00
,0x00,0x3f,0xa0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xc0,0x00,0x00,0x00,0x00
,0x00,0x16,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM bandoneon2[] =
{0x00,0x00,0x00,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x01,0xf5,0x2e,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0xd8,0x00,0x00,0x00
,0x00,0x00,0x39,0x12,0x92,0xe0,0x00,0x00,0x00,0x00,0x0f,0x88,0x43,0xf8,0x00,0x00
,0x00,0x02,0xc8,0x89,0x49,0x4e,0x07,0xf0,0x03,0xf3,0xd2,0x44,0x21,0x8a,0x00,0x00
,0x00,0x08,0x44,0x44,0x20,0xa9,0x40,0x47,0xf8,0x08,0x11,0x00,0x20,0x89,0x00,0x00
,0x00,0x70,0x22,0x22,0x80,0x04,0x22,0x24,0x80,0x80,0x49,0x22,0x10,0xc4,0x00,0x00
,0x00,0x48,0x11,0x11,0x42,0x10,0x81,0x22,0x48,0x04,0x48,0x81,0x00,0x40,0x80,0x00
,0x00,0x84,0x04,0x88,0xa8,0x2a,0x40,0x00,0x40,0x40,0x04,0x10,0x08,0x62,0x40,0x00
,0x00,0x82,0x02,0x40,0x18,0x01,0x48,0x91,0x24,0x42,0x00,0x40,0x84,0x61,0x00,0x00
,0x00,0xc1,0x01,0x00,0x58,0x29,0x24,0x01,0x02,0x20,0x02,0x48,0x00,0x30,0x20,0x00
,0x00,0x20,0x80,0x83,0x18,0x29,0x24,0x08,0x90,0x22,0x02,0x24,0x42,0x30,0x90,0x00
,0x00,0x10,0x00,0x29,0x1a,0x29,0x24,0x80,0x02,0x22,0x20,0x24,0x02,0x3b,0xd0,0x00
,0x00,0x48,0x00,0x21,0x1a,0x28,0x00,0x84,0x02,0x00,0x00,0x24,0x42,0x10,0x10,0x00
,0x00,0x5c,0x01,0x81,0x12,0x28,0x00,0x84,0x02,0x00,0x02,0x24,0x40,0x10,0x90,0x00
,0x00,0x3a,0x06,0x01,0x12,0x28,0x00,0x84,0x00,0x44,0x42,0x24,0x44,0x20,0x70,0x00
,0x00,0x31,0xc4,0x05,0x12,0x00,0x48,0x00,0x00,0x44,0x42,0x04,0x44,0x21,0x60,0x00
,0x00,0x74,0x00,0x05,0x12,0x02,0x48,0x00,0x04,0x44,0x40,0x04,0x04,0x01,0xd0,0x00
,0x00,0x64,0xe0,0x05,0x12,0x02,0x48,0x08,0x04,0x00,0x00,0x04,0x00,0x43,0xd0,0x00
,0x00,0x68,0xbc,0x15,0x12,0x02,0x41,0x08,0x04,0x08,0x04,0x04,0x88,0x43,0xe0,0x00
,0x00,0x78,0xfe,0x15,0x12,0x10,0x01,0x08,0x00,0x88,0x04,0x04,0x88,0x83,0xc0,0x00
,0x00,0x7a,0x7e,0x15,0x12,0x50,0x01,0x00,0x00,0x88,0x84,0x04,0x88,0x87,0x80,0x00
,0x00,0x7a,0x7e,0x11,0x12,0x50,0x11,0x00,0x80,0x80,0x80,0x04,0x80,0x07,0x00,0x00
,0x00,0x48,0x7e,0x11,0x12,0x50,0x90,0x10,0x88,0x10,0x80,0x04,0x01,0x0f,0x00,0x00
,0x00,0x4c,0x7e,0x01,0x02,0x54,0x90,0x10,0x88,0x10,0x08,0x04,0x11,0x0e,0x00,0x00
,0x00,0x1d,0x7e,0x01,0x02,0x44,0x92,0x00,0x89,0x10,0x08,0x05,0x12,0x1e,0x00,0x00
,0x00,0x2d,0x3e,0x01,0x1f,0x44,0x82,0x00,0x81,0x01,0x08,0x05,0x02,0x1c,0x00,0x00
,0x00,0x1d,0x3e,0x03,0x60,0x34,0x02,0x27,0xf1,0x21,0x00,0x05,0x20,0x2c,0x00,0x00
,0x00,0x0f,0x3f,0x0f,0x00,0x04,0x2f,0xc0,0x07,0xa1,0x00,0x73,0xff,0x20,0x00,0x00
,0x00,0x00,0xbf,0x10,0x00,0x01,0x80,0x00,0x00,0x1f,0x1e,0x00,0x00,0x20,0x00,0x00
,0x00,0x00,0x7d,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x2f,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM bandoneon3[] =
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x3a,0x9e,0x00,0x00,0x00,0x00,0x42,0x60,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x0e,0xc9,0x49,0x70,0x00,0x00,0x7e,0x00,0x0f,0xc0,0x00,0x00,0x00
,0x00,0x00,0x00,0xf4,0x44,0x24,0xa6,0x00,0xce,0x82,0x21,0x0c,0x50,0x00,0x00,0x00
,0x00,0x00,0x06,0x22,0x22,0x10,0x54,0xf1,0x00,0x09,0x10,0x86,0x28,0x00,0x00,0x00
,0x00,0x00,0x18,0x11,0x11,0x40,0x02,0x11,0x22,0x40,0x00,0x86,0x24,0x00,0x00,0x00
,0x00,0x00,0x24,0x08,0x88,0xa1,0x20,0x40,0x00,0x04,0x88,0x42,0x10,0x00,0x00,0x00
,0x00,0x00,0x42,0x02,0x40,0x54,0x15,0x20,0x91,0x20,0x04,0x03,0x02,0x00,0x00,0x00
,0x00,0x00,0x41,0x01,0x20,0x08,0x00,0x84,0x01,0x02,0x40,0x21,0x89,0x00,0x00,0x00
,0x00,0x00,0x60,0x80,0x80,0x04,0x00,0x12,0x08,0x11,0x02,0x11,0x84,0x00,0x00,0x00
,0x00,0x00,0x10,0x40,0x41,0x94,0x10,0x92,0x00,0x01,0x20,0x00,0xc0,0x80,0x00,0x00
,0x00,0x00,0x28,0x00,0x10,0x84,0x14,0x92,0x11,0x09,0x21,0x00,0xde,0xc0,0x00,0x00
,0x00,0x00,0x24,0x00,0x10,0x80,0x14,0x92,0x11,0x01,0x20,0x10,0x82,0x00,0x00,0x00
,0x00,0x00,0x2a,0x00,0x62,0x88,0x14,0x80,0x11,0x01,0x20,0x10,0x80,0xc0,0x00,0x00
,0x00,0x00,0x1d,0x03,0x02,0x88,0x14,0x00,0x00,0x01,0x22,0x00,0x05,0xc0,0x00,0x00
,0x00,0x00,0x1a,0xfe,0x02,0x88,0x14,0x00,0x20,0x11,0x22,0x01,0x07,0x40,0x00,0x00
,0x00,0x00,0x1a,0x40,0x0a,0x88,0x14,0x24,0x22,0x11,0x22,0x21,0x0f,0x00,0x00,0x00
,0x00,0x00,0x3e,0x40,0x0a,0x89,0x11,0x24,0x22,0x11,0x22,0x22,0x0e,0x80,0x00,0x00
,0x00,0x00,0x34,0x7b,0x0a,0x89,0x01,0x24,0x42,0x01,0x20,0x02,0x1f,0x00,0x00,0x00
,0x00,0x00,0x2c,0x7f,0x08,0x89,0x01,0x24,0x40,0x01,0x20,0x00,0x1e,0x00,0x00,0x00
,0x00,0x00,0x29,0x3f,0x08,0x89,0x01,0x00,0x80,0x21,0x24,0x44,0x1c,0x00,0x00,0x00
,0x00,0x00,0x2d,0x3f,0x00,0x89,0x09,0x00,0x80,0x21,0x24,0x44,0x38,0x00,0x00,0x00
,0x00,0x00,0x2e,0x3f,0x00,0x89,0x08,0x08,0x84,0x21,0x24,0x08,0x38,0x00,0x00,0x00
,0x00,0x00,0x2e,0x3f,0x00,0x89,0x28,0x48,0x04,0x01,0x24,0x08,0x70,0x00,0x00,0x00
,0x00,0x00,0x0e,0xbf,0x01,0x89,0x28,0x48,0x04,0x01,0x20,0x88,0x70,0x00,0x00,0x00
,0x00,0x00,0x16,0x9f,0x81,0x8b,0x2a,0x48,0x00,0x41,0x20,0x80,0xe0,0x00,0x00,0x00
,0x00,0x00,0x0e,0x1f,0x85,0xb8,0x1a,0x41,0x00,0x41,0x28,0x80,0xe0,0x00,0x00,0x00
,0x00,0x00,0x07,0x9f,0x87,0xc0,0x06,0x41,0x08,0x43,0xcf,0xf9,0x40,0x00,0x00,0x00
,0x00,0x00,0x00,0x5f,0x84,0x00,0x00,0xc0,0x38,0xf0,0x00,0x01,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x3e,0x90,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x17,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x0f,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM bandoneon4[] =
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x0a,0x0e,0x03,0xc4,0x01,0xf8,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x07,0xc9,0x49,0xf9,0x04,0x20,0x8a,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0xf4,0x44,0xa4,0xa9,0x22,0x10,0xc5,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x02,0x22,0x22,0x12,0x44,0x80,0x00,0x40,0x80,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x0c,0x11,0x11,0x08,0x24,0x91,0x08,0x62,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x24,0x08,0x88,0xa5,0x00,0x00,0x80,0x21,0x40,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x02,0x44,0x50,0x92,0x48,0x04,0x31,0x20,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x60,0x01,0x22,0x0a,0x82,0x24,0x42,0x30,0x80,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x20,0x00,0x80,0x06,0x81,0x24,0x20,0x18,0x10,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x30,0x20,0x40,0x96,0x81,0x00,0x20,0x1f,0xc8,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x28,0x10,0x12,0x96,0x81,0x00,0x20,0x18,0x48,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x24,0x08,0x0a,0x92,0x91,0x00,0x02,0x10,0x08,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x0e,0x04,0x2a,0x92,0x91,0x00,0x02,0x10,0xb8,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x1f,0x02,0x8a,0x92,0x91,0x20,0x00,0x20,0xf8,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x1e,0xff,0x08,0x92,0x93,0x20,0x40,0x21,0xe8,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x1e,0x41,0x08,0x92,0x92,0x20,0x44,0x21,0xd0,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x1c,0x41,0x08,0x92,0x02,0x20,0x44,0x41,0xc0,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x1d,0x3b,0x00,0x92,0x02,0x20,0x00,0x43,0xe0,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x29,0x3f,0x00,0x12,0x02,0x20,0x00,0x03,0x80,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x29,0x3f,0x01,0x12,0x02,0x20,0x08,0x87,0x80,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x2e,0x3f,0x01,0x12,0x22,0x20,0x88,0x87,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x0e,0xbf,0x01,0x12,0x20,0x20,0x81,0x0f,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x0e,0x9f,0x85,0x1a,0x20,0x20,0x81,0x0e,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x12,0x9f,0x85,0x1b,0x20,0x20,0x10,0x0e,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x16,0x1f,0x81,0x1b,0x24,0x24,0x10,0x14,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x0f,0x5f,0x84,0x1c,0x3f,0x9f,0xfe,0x08,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x07,0xdf,0x85,0xe0,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x01,0xdf,0x86,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x2f,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x1f,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x0f,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM bandoneon5[] =
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x53,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x07,0xe0,0x69,0x07,0x18,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x13,0x00,0x24,0x85,0x82,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x40,0x81,0x12,0x61,0x89,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0x20,0x44,0x89,0x32,0x84,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x02,0x10,0x22,0x44,0x90,0xc0,0x80,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x02,0x08,0x11,0x22,0x19,0x42,0x40,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x03,0x04,0x00,0x91,0x49,0x61,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x82,0x02,0x49,0x24,0x21,0x20,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x41,0x01,0x25,0x02,0x38,0x80,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0x20,0x80,0x85,0x00,0x30,0xf0,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0x50,0x41,0x85,0x05,0x20,0x90,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0xe8,0x24,0x05,0x25,0x20,0x30,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0xe4,0xf0,0x15,0xa4,0x21,0x70,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0xd2,0x00,0x55,0xa0,0x41,0xd0,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0xd2,0x00,0x55,0xaa,0x43,0xd0,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0xb3,0xc0,0x55,0xaa,0x83,0xa0,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0xe3,0xf8,0x55,0xa8,0x87,0xc0,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0x69,0xf8,0x45,0x80,0x87,0x80,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0x69,0xf8,0x45,0x95,0x07,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0x69,0xf8,0x45,0xd5,0x0e,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x01,0x31,0xf8,0x05,0xd2,0x0e,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x75,0xf8,0x05,0xc2,0x1c,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0xf5,0xf8,0x0d,0xca,0x1c,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0xb4,0xfc,0x0d,0xe8,0x3c,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x7c,0xfc,0x2f,0xfc,0x28,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x1e,0xfc,0x30,0x03,0xf0,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0xf4,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x01,0xbd,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM bandoneon6[] =
{0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x71,0x13,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x04,0xd8,0x89,0x31,0xc0,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x10,0x4c,0x44,0x90,0x20,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x50,0x26,0x20,0x18,0x90,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x88,0x13,0x10,0x48,0x50,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x04,0x05,0x89,0x2c,0x08,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x82,0x02,0xc0,0x94,0x20,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0xc1,0x01,0x60,0x56,0x04,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0xa0,0x80,0xf0,0x42,0x12,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x90,0x00,0x38,0x13,0xca,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x78,0x12,0x0e,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x6c,0x03,0x28,0x12,0x03,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x7a,0x0c,0x29,0x82,0x17,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x79,0x04,0x29,0xa4,0x1d,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x71,0x04,0x29,0xa4,0x3d,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x74,0xe4,0x09,0xa0,0x3a,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x24,0xfc,0x09,0x88,0x78,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x24,0xfc,0x0d,0x48,0x7c,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x38,0xfc,0x1d,0x50,0x78,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x3a,0xfc,0x0d,0x50,0xe0,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x3a,0x7e,0x1d,0x00,0xe0,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x4a,0x7e,0x1d,0x21,0xe0,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x58,0x7e,0x0d,0xa1,0xc0,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x3d,0x7e,0x19,0x83,0xc0,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x1d,0x7e,0x15,0x03,0x80,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x0f,0x3e,0x1f,0xc1,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0xbe,0x20,0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM IniciandoBanonberry[] =
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00
,0x07,0xfe,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00
,0x07,0xff,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x0e,0x00,0x00,0x00,0x00,0x00,0x00
,0x07,0x9f,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x0e,0x00,0x00,0x00,0x00,0x00,0x00
,0x07,0x8f,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x0e,0x00,0x00,0x00,0x00,0x00,0x00
,0x07,0x8f,0x1f,0x63,0x9e,0x1e,0xf1,0xf8,0xf7,0x8f,0xf8,0x7e,0x39,0xb9,0x38,0xe0
,0x07,0xfe,0x3f,0xf3,0xfe,0x3f,0xe3,0xfc,0xff,0xcf,0xfc,0xff,0x3f,0xbf,0x78,0xe0
,0x07,0xfe,0x3f,0xf3,0xff,0x7f,0xe7,0xfc,0xff,0xcf,0xfc,0xe7,0xbf,0xbf,0x78,0xe0
,0x07,0xff,0x78,0xf3,0xcf,0x78,0xe7,0x9e,0xf3,0xcf,0x3c,0xe7,0xbf,0xbf,0x78,0xe0
,0x07,0x8f,0x78,0xf3,0x8f,0x70,0xe7,0x0e,0xf1,0xce,0x1f,0xff,0xbc,0x3c,0x78,0xe0
,0x07,0x8f,0x78,0xf3,0x8f,0x78,0xf7,0x0e,0xf1,0xce,0x1f,0xfe,0x3c,0x3c,0x3d,0xe0
,0x07,0x8f,0x78,0xf3,0x8f,0x79,0xf7,0x9e,0xf1,0xcf,0x3c,0xe0,0x3c,0x3c,0x3f,0xe0
,0x07,0xff,0x3f,0xf3,0x8f,0x7f,0xf7,0xfc,0xf1,0xcf,0xfc,0xff,0x3c,0x3c,0x1f,0xc0
,0x07,0xfe,0x3f,0xf3,0x8f,0x3f,0xf3,0xfc,0xf1,0xcf,0xf8,0x7f,0x3c,0x3c,0x0f,0x80
,0x07,0xfc,0x0e,0xf3,0x8e,0x0e,0xf0,0xf0,0x71,0xce,0xf0,0x3e,0x3c,0x38,0x0f,0x80
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3c,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM Bateria[] =
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x1f,0x87,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x1f,0x87,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x1f,0x87,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xff,0xff,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xff,0xff,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xc0,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xc0,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xc0,0x03,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xc0,0x03,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xcf,0xcf,0xcc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xcf,0xcf,0xcc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xc0,0x03,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xc0,0x03,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xc0,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xc0,0x00,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xff,0xff,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0xff,0xff,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

////////////////////////////////////  FIN DE IMAGENES PARA ANIMACIONES  ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////  SETUP  ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup(){
    // Defino input
    pinMode(ShutDownBot_IN, INPUT);         // Activas por nivel bajo
    pinMode(RaspState_IN, INPUT);           // Activas por nivel bajo
    
    // Defino output
    pinMode(PowerOnOff_OUT, OUTPUT);
    pinMode(RaspOff_OUT, OUTPUT);
    pinMode(ROn_OUT, OUTPUT);
    
    // Incializacion de salidas
    digitalWrite(PowerOnOff_OUT, HIGH);     // Sistema con corriente pasando
    digitalWrite(RaspOff_OUT, LOW);         // No se envia ninguna orden a la raspberry
    
    // Setup display
    delay(2000);                                // Le da tiempo al dispay que inicialice si se saca este delay el display no anda!
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize with the I2C addr 0x3C (for the 128x32)

//    display.display();
//    display.clearDisplay();
//    display.setTextSize(2);
//    display.setTextColor(WHITE);
//    display.setCursor(0,0);
//    display.println("Iniciando");
//    display.display();
//    delay(2000);

    display.clearDisplay();
    display.drawBitmap(0, 0, IniciandoBanonberry, 128, 32, 1);
    display.display();
    delay(3000); 
    
//    CheckDivices();
    
    // Setup Fuel Gauge
//    batteryMonitor.reset();
//    batteryMonitor.quickStart();
//    delay(2500);
    
    CheckBat();                                  // Chequeo estado de la bateria antes de energizar Raspberry
    
    digitalWrite(ROn_OUT, HIGH);                 // Enciendo Raspberry    
}

////////////////////////////////////  FIN SETUP  ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////  MAIN  ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop(){

    ////////////////////////////////// INICIO //////////////////////////////////
    
    StartTime = millis();   // Inicio de time out de raspberry , prevencion de error de ausencia de señal de fallo de RState
    
    while (RState == LOW){  // Hasta que no esta el boot ok me quedo aca
    
        // Prevencion de error de ausencia de señal de fallo de RState
        
        ResultTime = millis() - StartTime;     // Mido cuanto tiempo paso sin estar pronto el booteo de la raspberry
        
        if (ResultTime > Time_Out_Boot_Raspb){ // Si la raspberry no bootea se apaga todo 
            error=1;
            FatalErrorList();
        }
        
        // Animaciones en el tiempo muerto de booteo de la raspberry
        
        display.clearDisplay();
        display.drawBitmap(0, 0,  bandoneon6, 128, 32, 1); //(ubicacion x, ubicacion y, nombre de la imagen, tamano x, tamano y, color)
        display.display();
        delay(400);
        display.clearDisplay();
        display.drawBitmap(0, 0,  bandoneon5, 128, 32, 1); //(ubicacion x, ubicacion y, nombre de la imagen, tamano x, tamano y, color)
        display.display();
        delay(200);
        display.clearDisplay();
        display.drawBitmap(0, 0,  bandoneon4, 128, 32, 1); //(ubicacion x, ubicacion y, nombre de la imagen, tamano x, tamano y, color)
        display.display();
        delay(200);
        display.clearDisplay();
        display.drawBitmap(0, 0,  bandoneon3, 128, 32, 1); //(ubicacion x, ubicacion y, nombre de la imagen, tamano x, tamano y, color)
        display.display();
        delay(300);
        display.clearDisplay();
        display.drawBitmap(0, 0,  bandoneon4, 128, 32, 1); //(ubicacion x, ubicacion y, nombre de la imagen, tamano x, tamano y, color)
        display.display();
        delay(200);
        display.clearDisplay();
        display.drawBitmap(0, 0,  bandoneon5, 128, 32, 1); //(ubicacion x, ubicacion y, nombre de la imagen, tamano x, tamano y, color)
        display.display();
        delay(200);
        
        // Fin de animaciones
        
        RState = digitalRead(RaspState_IN); // Chquea si la señal de pronto para apagar esta OK. Si esta OK, el Rstate es LOW.
    }
    
    ////////////////////////////////// REPOSO //////////////////////////////////
    
    BState = digitalRead(ShutDownBot_IN);       // Para asegurarme que el estado del boton quede en HIGH
    
    while (BState == HIGH){                     // Este while hace un loop cerrado hasta que se apriete el boton
        
        BState = digitalRead(ShutDownBot_IN);   // Hasta que no se apriete el boton no sigo

        if (CheckBat()){
          // Muestro estado de la bateria
          display.clearDisplay();
          
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          
          display.print("     ");
          float v_cell = batteryMonitor.getVCell();
          display.print(v_cell);
          display.println("V");
          
          int soc = batteryMonitor.getSoC();
          if (soc < 100){
            display.print(" ");
          }
          if (soc < 10){
            display.print(" ");
          }
          display.print("      ");
          display.print(soc);
          display.print("%");
  
          display.drawBitmap(0, 0, Bateria, 128, 32, 1);
          
          display.display();
        }    
        
    }

    ////////////////////////////////// APAGADO //////////////////////////////////
    
    StartTime = millis();                     // Mido cuanto tiempo en el que se apreto el boton
    ResultTime = millis() - StartTime;        // Inicializo ResultTime
    
    while ((BState == LOW)&&(ResultTime < 10000)){
        BState = digitalRead(ShutDownBot_IN);       // Hasta que no se suelte el boton no sigo
        ResultTime = millis() - StartTime;          // Mido cuanto tiempo en el que se solto el boton
    }
    
    if (ResultTime >= 10000){
        ApagadoForzado();
    } else {
        ApagadoNormal();
    }
}
       
////////////////////////////////////  FIN MAIN  ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////  FUNCIONES  ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CheckDivices(){
    byte error, address;
    int nDevices = 0;           // Numero de perifericos
    
    display.clearDisplay();     // Para sacar el "Iniciando"
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    
    for(address = 1; address < 127; address++){
        
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0){
            // No ponemos clear display para que entre todo
            display.print("Direccion 0x");
            display.println(address,HEX);
            display.display();
         
            if (address == 0x3C){
                display.println("Display OK!");  
            } else if (address == 0x36){
                display.println("Fuel Gauge OK!");
            } else {
                display.println("ERROR!");
            }
            display.display();
            nDevices++;
        } else if (error == 4){
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            display.print("Direccion 0x");
            display.print(address,HEX);
            display.println("ERROR");
            display.display();
            delay(2000);
        }    
    }
    
    if (nDevices == 0){
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("No se encontraron");
        display.print("dispositivos! ERROR");
        display.display();
        delay(2000);
    } else {
        // No ponemos clear display para que entre todo
        display.println("Todo OK!");
        display.display();
    }
    delay(3000);
}                     

void ApagadoNormal(){
    // Muestro en pantalla apagado normal
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Apagando");
    display.display();
    delay(1000);    
    
    digitalWrite(RaspOff_OUT, HIGH);    // Se envia orden a raspberry
    delay(1000);    
    digitalWrite(RaspOff_OUT, LOW);     // Por el tiempo del HIGH es un apagado normal
    
    StartTime = millis(); // Inicio de time out de raspberry, prevencion de error de ausencia de señal de fallo de RState              
    while (RState == HIGH){
        RState = digitalRead(RaspState_IN); // Chquea si la señal de pronto para apagar esta OK. Si esta OK, el Rstate es LOW
        
        // Prevencion de error de ausencia de señal de fallo de RState
        ResultTime = millis() - StartTime;  // Mido cuanto tiempo paso sin estar pronto el booteo de la raspberry
        
        if (ResultTime > Time_Out_Boot_Raspb){ // Si la raspberry no bootea se apaga todo 
            error = 2;
            FatalErrorList();
        }
    }
    
    delay(12000);
    digitalWrite(ROn_OUT, LOW); // Desenergizo la raspberry un segundo antes que a todo el sistema
    delay(1000);
    
    display.clearDisplay();
    display.display();
    
    digitalWrite(PowerOnOff_OUT, LOW);
    delay(3000); // Para que el display no cambie hasta que se genere la accion de apagado o forzado

    // No deberia llegar hasta este punto
    error = 3;
    FatalErrorList();
}

void ApagadoForzado(){
    // Muestro en pantalla apagado forzado
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Apagado");
    display.print("forzado");
    display.display();
    delay (2000);
    
    while(digitalRead(ShutDownBot_IN) == LOW);
    
    digitalWrite(PowerOnOff_OUT, LOW);
    delay(3000); // Para que el display no cambie hasta que se genere la accion de apagado o forzado

    // No deberia llegar hasta este punto
    error = 3;
    FatalErrorList();
}

bool CheckBat(){
    if (batteryMonitor.getVCell() < Umbral_Low_Bat){

        if (digitalRead(ROn_OUT) || (batteryMonitor.getVCell() < Umbral_Dead_Bat)){
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println("Conecte el");
          display.print("cargador");
          display.display(); 
        }               
        
        if (batteryMonitor.getVCell() < Umbral_Dead_Bat){
            delay(3000);
            ApagadoNormal();
            // No debe llegar a este punto
        }
        return false;
    }
    return true;
}

void FatalErrorList(){
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("Error ");
    switch (error) {
      case 1:  display.print("1"); break;  // No se obtuvo la señal de boot OK de la Raspberry al Arduino. Se apagará todo el sistema segundos despues de mostrar el mensaje de error.
      case 2:  display.print("2"); break;  // BRasp_Off No se obtuvo la señal de finializacion del boot OK de la Raspberry al Arduino. Se apagará todo el sistema segundos despues de mostrar el mensaje. de error
      case 3:  display.print("3"); break;  // Por algún motivo la señal de apagado no funciona, o bien no se envía correctamente, o bien el circuito de desconexión tiene algún problema.
      default: display.print("4"); break;  // Error desconocido
    }
    display.display();
    delay(10000);
    
    digitalWrite(PowerOnOff_OUT, LOW);
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("Error 5");
    display.display();
    while(1);
}
