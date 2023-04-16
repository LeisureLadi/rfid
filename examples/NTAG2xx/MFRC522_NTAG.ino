/*
 * This example show how you deal with NTAG213/215/216 tags. By default the tags are unprotected. I order to protect them please consult the corresponding data sheet.
 * @license Released into the public domain.
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN   D3    
#define SS_PIN    D8    

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
MFRC522::StatusCode status;       // define variable to hold the return value of MFRC522 functions

bool cardIsPresent = false;   // required to detect, when a card was removed from reader


/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}


/**
 * Setup
 */
void setup() {
  Serial.begin(115200);   // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522

  delay(1000);
  Serial.println();

  // do a self test  
  // --------------
  Serial.print(F("Selftest "));
  Serial.println((mfrc522.PCD_PerformSelfTest()) ? "passed" : "failed");
  mfrc522.PCD_DumpVersionToSerial();
  // --------------
  
  Serial.println(F("Scan PICC to see UID, type, and data blocks..."));
}

void loop() {

  // check if card is still present (but check only, if card was registered already  
  // ------------------------------------------------------------------------------
  if (cardIsPresent) {
    cardIsPresent = mfrc522.PICC_IsCardStillPresent();
  } else {

    // card is not present any more or wasn't registered yet  
    // -----------------------------------------------------
    
    // check if a new card was presented (return since there is nothing to do in this demo)
    if ( ! mfrc522.PICC_IsNewCardPresent())
      return;

    // check if a new card can be registered
    if ( ! mfrc522.PICC_ReadCardSerial())
      return;

    // remember that this card was registered
    cardIsPresent = true;


/**
 * Demonstrate some general functions
 */
    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    Serial.print(F("SAK: "));
    Serial.println(mfrc522.uid.sak, HEX);
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // NTAG cards are in general identified as MIFARE Ultarlight, so it is required to check, if we can read NTAG's version bytes
    if (piccType == MFRC522::PICC_TYPE_MIFARE_UL) {

      
/**
 * Demonstrate NTAGxx_GETVERSION & NTAG2XX_GetType
 */
      byte verbuffer[8];
      status = (MFRC522::StatusCode) mfrc522.NTAG2XX_GETVERSION(verbuffer, 8);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("NTAG2XX_GETVERSION failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
      } else {
        piccType = mfrc522.NTAG2XX_GetType(verbuffer);
        Serial.print(F("PICC type: "));
        Serial.println(mfrc522.PICC_GetTypeName(piccType));
      }
      Serial.print(F("VERSION: "));
      dump_byte_array(verbuffer, 8);
      Serial.println();  
    }


/**
 * Demonstrate NTAGxx_GETSIGNATURE
 */
    byte signature[32];
    status = (MFRC522::StatusCode) mfrc522.NTAG2XX_GETSIGNATURE(signature, 32);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("NTAG2XX_GETSIGNATURE failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("SIGNATURE: "));
    dump_byte_array(signature, 32);
    Serial.println();  


/**
 * Demonstrate NTAGxx_AUTH (pACK should be verified after authentication against the programmed value) 
 */
    byte PWD[] = {0xFF, 0xFF, 0xFF, 0xFF}; // 32 bit password default is FFFFFFFF. Enter your own password here if it was set.
    byte pACK[] = {0, 0}; // 16 bit password ACK returned by the NFCtag.

    status = (MFRC522::StatusCode) mfrc522.NTAG2XX_AUTH(PWD, pACK);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("NTAG2XX_AUTH failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println(F("NTAG2XX_Authentication succeeded"));
    Serial.print(F("returned PACK: "));
    dump_byte_array(pACK, 2);
    Serial.println();


/**
 * Demonstrate NTAGxx_Write (copy of MIFARE_Ultralight_Write / added for convenience only) 
 */
    byte pageAddr      = 6;
    byte BuffWrite[] = {0xAF, 0xFE, 0xBE, 0xEF}; // 32 bit data.

    // Write data to the block
    Serial.println(F("Writing data into block "));
    Serial.print(pageAddr);
    Serial.print(F(" ... "));
    dump_byte_array(BuffWrite, 4);
    Serial.println();
    status = (MFRC522::StatusCode) mfrc522.NTAG2XX_Write(pageAddr, BuffWrite, 4);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("NTAG2XX_Write failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }


/**
 * Demonstrate NTAGxx_Read4Pages (copy of MIFARE_Read / added for convenience only) 
 * (function name shall remind, that actually 4 pages are transferred!)
 */
    byte buffer[18];
    byte size = sizeof(buffer);

    Serial.println(F("Reading data from page "));
    // Read data from the block
    for (pageAddr = 4; pageAddr < 10; pageAddr++) {

      Serial.print(pageAddr);
      Serial.print(F(" ... "));
      status = (MFRC522::StatusCode) mfrc522.NTAG2XX_Read4Pages(pageAddr, buffer, &size);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("NTAG2XX_Read4Pages failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
      }
      Serial.print(F(": "));
      dump_byte_array(buffer, 16);
      Serial.println();
    }


/**
 * Demonstrate NTAGxx_FastRead 
 */
    byte fastbuffer[68];        // MFRC522 can handle buffers up to 64 bytes + 4 bytes for CRC and PICC commands
    byte fastsize = sizeof(fastbuffer);

    Serial.println(F("Reading 15 pages from page ")); // that is the max. number of pages we can read with MFRC522
    // Read data from start page 4
    pageAddr = 4;
  
    Serial.print(pageAddr);
    Serial.print(F(" ... "));
    status = (MFRC522::StatusCode) mfrc522.NTAG2XX_FastRead(pageAddr, 15, fastbuffer, &fastsize);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("NTAG2XX_FastRead failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F(": "));
    dump_byte_array(fastbuffer, 60);
    Serial.println();
  

/**
 * Demonstrate NTAGxx_GETCOUNTER (for details on this counter, please consult the NTAG data sheet) 
 */
    byte counter[4];
    status = (MFRC522::StatusCode) mfrc522.NTAG2XX_GETCNT(counter, 4);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("NTAG2XX_GETCOUNTER failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("CNT: "));
    dump_byte_array(counter, 4);
    Serial.println();  





/**
 * After card was processed, it is recommended to put the card into "halt" state, so that we can check, if it is still present
 */
    status = (MFRC522::StatusCode) mfrc522.PICC_HaltA();
    delay (100);      // slow down the loop a bit
  }
}
