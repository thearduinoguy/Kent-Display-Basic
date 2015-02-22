/*
* Kent 320x240 LCD Driver
*/

#define DATAOUT 11 //MOSI (SI line on Sparkfun breakout board)
#define DATAIN 12 //MISO (SO line on Sparkfun breakout board)
#define SPICLOCK 13 //sck (SCK line on Sparkfun breakout board)
#define SLAVESELECT 10 //ss (CS line on Sparkfun breakout board)
#define BUSY 9 //Busy (BUSY line on Sparkfun breakout board)

char clr = 0;

void setup()
{
//Initialize I/O pins
pinMode(DATAOUT, OUTPUT);
pinMode(DATAIN, INPUT);
digitalWrite(DATAIN,LOW);
pinMode(SPICLOCK,OUTPUT);
pinMode(SLAVESELECT,OUTPUT);
digitalWrite(SLAVESELECT,HIGH);
pinMode(BUSY, INPUT);
digitalWrite(BUSY, LOW);
delay(1000);
}

void loop()
{
//SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << SPR1); // Initialize SPI communication
SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<CPHA);
clr=SPSR;
clr=SPDR;
/*
* Driver example code
*/
int std_delay = 25;

delay(std_delay); //Discovered that some delay is needed after setting SCPCR
FILL(0x00,0x00,0x25,0x7F,0xFF); //Clear entire screen buffer (bright)
delay(125); //Fill command seems to take a long time to process, this is where a busy() wait function would be useful

WRITE(0x00,0x29,0x55); //Write 01010101 to the first byte in the active area of the display's RAM
WRITEmore(0x66); //Write 01100110 to the second byte in the display's RAM (not necessary but shows how to use WRITEmore)
WRITEend(); //Since WRITE is a variable length command we must invoke the end function
delay(std_delay);

char RecievedData = 0x00; //Create char to store from display's RAM read

RecievedData = READ(0x00,0x2A); //Read second char from display's RAM
READend(); //Since READ is a variable length command we must invoke the end function
delay(std_delay);

FILL(0x25,0x31,0x25,0x56,RecievedData); //Fill last row of display active area with the byte that was read in (should be 01100110)
delay(std_delay);

DISP_FULLSCRN(0x00,0x00);
delay(5000); //To let the viewer see the updated screen, the command actually took about 1.63 seconds to run at room temperature

CLR_DISP_DRK(); //Use one of the CLR commands to demonstrate how much faster they run
delay(5000);

CLR_SECT_BRT(0x40,0x80); //Another of the CLR commands the clears based on row number, clearing rows 64 to 128
delay(5000);

DISP_FULLSCRN(0x00,0x00); //Refresh the display from RAM, showing that the information is still there (it is lost only if power is lost)

while(1) {
}
}

/*
* Functions below are the basic commands available listed in the Kent 320x240 Display Datasheet (25085b_320x240_SPI_datasheet.pdf)
* available at: http://www.kentdisplays.com/services/resources/datasheets/25085b_320x240_SPI_datasheet.pdf
*/

//Writes data to screen RAM starting at the target memory address, variable length command
//Note: the WRITEend() function must be called after sending the variable amount of Data Bytes
void WRITE(int HighAddress, int LowAddress, int Data) {
select();
spi_transfer(0x00); //Write Command
spi_transfer(HighAddress); //High byte of the target memory address
spi_transfer(LowAddress); //Low byte of the target memory address
spi_transfer(Data); //The first value to be written, more may follow
}

//Optional if more data needs to be sent
void WRITEmore(int Data) {
spi_transfer(Data); //Send additional byte to screen
}

//Deselect screen so that it knows there is no more data to be sent
void WRITEend() {
deselect();
}

//Fills multiple addresses in RAM with FillValue byte, starting with the First address and ending with the Last address, fixed length command
void FILL(int HighFirst, int LowFirst, int HighLast, int LowLast, int FillValue) {
select();
spi_transfer(0x01); //Fill Command
spi_transfer(HighFirst); //High byte of the first address in the fill region
spi_transfer(LowFirst); //Low byte of the first address in the fill region
spi_transfer(HighLast); //High byte of the last address in the fill region
spi_transfer(LowLast); //Low byte of the last address in the fill region
spi_transfer(FillValue); //Fill Value
deselect();
}

//Reads data from RAM starting with the target address, variable length command
//Note: the READend() function must be called after recieving the variable amount of Data Bytes
char READ(int HighAddress, int LowAddress) {
select();
spi_transfer(0x04); //Read Command
spi_transfer(HighAddress); //High byte of the target memory address
spi_transfer(LowAddress); //Low byte of the target memory address
spi_transfer(0x00); //Dummy byte
spi_transfer(0x00); //Dummy byte
return(spi_transfer(0x00)); //Final dummy byte, Screen will transfer data (on SO line) during reciept of this byte
}

//Optional if more data needs to be read
char READmore() {
return(spi_transfer(0x00)); //Additional dummy byte, Screen will transfer data (on SO line) during reciept of this byte
}

//Deselect screen so that it knows there is no more data to be read
void READend() {
deselect();
}

//Clear Bits sets bits to zero according to the Mask, a 1 in the mask affects that pixel, a 0 leaves the pixel unaffected, fixed length command
void CLEAR_BITS(int HighAddress, int LowAddress, int Mask) {
select();
spi_transfer(0x08); //Clear Bits Command
spi_transfer(HighAddress); //High byte of the target memory address
spi_transfer(LowAddress); //Low byte of the target memory address
spi_transfer(Mask); //Send Mask
deselect();
}

//Set Bits sets bits to one according to the Mask, a 1 in the mask affects that pixel, a 0 leaves the pixel unaffected, fixed length command
void SET_BITS(int HighAddress, int LowAddress, int Mask) {
select();
spi_transfer(0x09); //Set Bits Command
spi_transfer(HighAddress); //High byte of the target memory address
spi_transfer(LowAddress); //Low byte of the target memory address
spi_transfer(Mask); //Send Mask
deselect();
}

//XOR Bits toggles bits according to the Mask, a 1 in the mask affects that pixel, a 0 leaves the pixel unaffected, fixed length command
void XOR_BITS(int HighAddress, int LowAddress, int Mask) {
select();
spi_transfer(0x0A); //XOR Bits Command
spi_transfer(HighAddress); //High byte of the target memory address
spi_transfer(LowAddress); //Low byte of the target memory address
spi_transfer(Mask); //Send Mask
deselect();
}

//Clear Display Bright clears entire screen to be bright including the border, fixed length command
void CLR_DISP_BRT() {
select();
spi_transfer(0x10); //Clear Display Bright Command
deselect();
}

//Clear Display Bright Inverted Border clears entire screen to be bright excluding the border, fixed length command
void CLR_DISP_BRT_IB() {
select();
spi_transfer(0x11); //Clear Display Bright Inverted Border Command
deselect();
}

//Clear Display Dark clears entire screen to be dark including the border, fixed length command
void CLR_DISP_DRK() {
select();
spi_transfer(0x12); //Clear Display Dark Command
deselect();
}

//Clear Display Dark Inverted Border clears entire screen to be dark excluding the border, fixed length command
void CLR_DISP_DRK_IB() {
select();
spi_transfer(0x13); //Clear Display Dark Inverted Border Command
deselect();
}

//Clear Section Bright clears rows of the screen (max of 120 rows) to be bright including the border, fixed length command
void CLR_SECT_BRT(int FirstRow, int LastRow) {
select();
spi_transfer(0x14); //Clear Section Bright Command
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(FirstRow); //First row in the clear region
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(LastRow); //Last row in the clear region
deselect();
}

//Clear Section Bright Inverted Border clears rows of the screen (max of 120 rows) to be bright excluding the border, fixed length command
void CLR_SECT_BRT_IB(int FirstRow, int LastRow) {
select();
spi_transfer(0x15); //Clear Section Bright Inverted Border Command
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(FirstRow); //First row in the clear region
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(LastRow); //Last row in the clear region
deselect();
}

//Clear Section Dark clears rows of the screen (max of 120 rows) to be dark including the border, fixed length command
void CLR_SECT_DRK(int FirstRow, int LastRow) {
select();
spi_transfer(0x16); //Clear Section Dark Command
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(FirstRow); //First row in the clear region
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(LastRow); //Last row in the clear region
deselect();
}

//Clear Section Dark Inverted Border clears rows of the screen (max of 120 rows) to be dark excluding the border, fixed length command
void CLR_SECT_DRK_IB(int FirstRow, int LastRow) {
select();
spi_transfer(0x17); //Clear Section Dark Inverted Border Command
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(FirstRow); //First row in the clear region
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(LastRow); //Last row in the clear region
deselect();
}

//Display Fullscreen triggers a full screen update from a specified image buffer in the onboard image RAM, fixed length command
void DISP_FULLSCRN(int HighAddress, int LowAddress) {
select();
spi_transfer(0x18); //Display Fullscreen command
spi_transfer(HighAddress); //High byte of the target memory address
spi_transfer(LowAddress); //Low byte of the target memory address
deselect();
}

//Display Partscreen triggers a partial per row screen update from a specified image buffer in the onboard image RAM, fixed length command
void DISP_PARTSCRN(int HighAddress, int LowAddress, int FirstRow, int LastRow) {
select();
spi_transfer(0x19); //Display Partscreen command
spi_transfer(HighAddress); //High byte of the target memory address
spi_transfer(LowAddress); //Low byte of the target memory address
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(FirstRow); //First row in the clear region
spi_transfer(0x00); //Not enough rows to need high byte
spi_transfer(LastRow); //Last row in the clear region
deselect();
}

//Sleep puts the display module in the low power sleep mode, fixed length command
void SLEEP() {
select();
spi_transfer(0x20); //Sleep command
deselect();
}

//Reset triggers a software reset of the display module, fixed length command
void RESET() {
select();
spi_transfer(0x24); //Reset command
deselect();
}

/*
* Helpful notes:
* Screen has a 32kB RAM allowing for the storage of 3.33 full screen buffers
*/

/*
* Below are functions for the SPI interface
*/

void select() {
digitalWrite(SLAVESELECT,LOW);
}

void deselect() {
digitalWrite(SLAVESELECT,HIGH);
}

char spi_transfer(volatile char data)
{
SPDR = data; // Start the transmission
while (!(SPSR & (1<<SPIF))){ //Wait for the end of the transmission
}
return SPDR; // return the received byte
}
