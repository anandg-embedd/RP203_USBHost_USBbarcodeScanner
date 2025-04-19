
#include <Usb.h>
#include <usbhub.h>
#include <hidboot.h>
#include <Adafruit_Thermal.h>
#include <SoftwareSerial.h>

#define TIMOUT_FETCH_BARCODE_MS   500

#define TX_PIN 6  // Arduino TX pin -- RX on printer
#define RX_PIN 5  // Arduino RX pin -- TX on printer

SoftwareSerial printerConnection(RX_PIN, TX_PIN);
Adafruit_Thermal printer(&printerConnection);

USB Usb;
//USBHub Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> BarcodeScanner(&Usb);

class BarcodeRptParser : public KeyboardReportParser {
  private:
  long last_key_ms;
  bool code_avail = false;
  String barcode;
  void OnKeyDown(uint8_t mod, uint8_t key) {  
    uint8_t c = OemToAscii(mod, key);
    if (c) {
      //Serial.write(c);
      barcode += (char)c;
      last_key_ms = millis();
    }
  }
  void OnKeyUp(uint8_t mod, uint8_t key) {
    // Handle key release events if needed
  }
  public:
  String getBarcode()
  {
    String ret="";
    if((millis()-last_key_ms) > TIMOUT_FETCH_BARCODE_MS)
    {
      ret = barcode;
    }
    return ret;
  }
  void resetBarcode()
  {
    barcode.remove(0);
  }
};

uint8_t OemToAscii(uint8_t mod, uint8_t key) {
  if (key >= 0x04 && key <= 0x1D) { // 'a' to 'z'
    if (mod & 0x22) { // Shift key is pressed
      return key - 0x04 + 'A';
    } else {
      return key - 0x04 + 'a';
    }
  } else if (key >= 0x1E && key <= 0x27) { // '1' to '0'
    return key == 0x27 ? '0' : key - 0x1E + '1';
  }
  // Add more mappings as needed
  return 0; // Return 0 if no valid mapping found
}

BarcodeRptParser BcPrs;

void setup() {
  Serial.begin(115200);
  if (Usb.Init() == -1) {
    Serial.println("USB Host Shield initialization failed.");
    while (1); // halt
  }
  BarcodeScanner.SetReportParser(0, (HIDReportParser*)&BcPrs);
  Serial.println("Barcode Scanner ready to scan.");
  printerConnection.begin(9600); // Initialize SoftwareSerial for printer
  printer.begin(); // Initialize the printer
  printer.println();
  printer.println();
  printer.println("\tBarcode Scanner");    //RP203 Thermal Printer Initialized
  printer.println("\t---------------");    
  printer.println();
  printer.println();
}

void loop() {
  Usb.Task();
  if(BcPrs.getBarcode() != "")
  {
    printer.println( BcPrs.getBarcode());
    Serial.println(BcPrs.getBarcode());
    printer.println();
    printer.println();
    printer.println();
    BcPrs.resetBarcode();
  }
}
