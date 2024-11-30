//---------------------------------Definitions--------------------------------------
#define DATA "Haim & Shahaf"    				 	//Data to send         ||-------------------------------------------------------------||
#define SIZE 7			 		                    //Data size            || Able to change any data size between 0-12 and any duty cycle|| 
#define BIT_TIME 5 									//1/100 clk duty cycle ||-------------------------------------------------------------||
//---------------------------------pins---------------------------------------------
#define txData 5                                    //Tranfer data pin
#define rxData 6									//Reciever data pin
#define txClk 7										//Tranfer clk pin
#define rxClk 8										//Reciever clk pin
//------------------------------Time function variables-----------------------------
unsigned long currentTX = 0;						//keep the current time in the tranfer
//---------------------------------Layer 1 Variables--------------------------------
uint8_t dataToSendL1 = 0;							//data to send value
boolean clkVal = false;          					//clk value
unsigned int recievedData=0;   					    //initialize data to recieve
int index = 0;										//index of the sent data
int count = 0;									    //size of the recieved data
unsigned long current = 0;						    //keep the current time
bool flagIfRead = false;                            //flag to check if digit is already read
bool tx_l1_busy = false;							//flag to check if the transfer of layer 1 is already read
bool rx_l1_busy = false;							//flag to check if the reciever of layer 1 is already read
bool msbOrLsbFlag = false;							//flag to check if msb sent
//---------------------------------Layer 2 Variables--------------------------------
char * dataToSendL2 = DATA;  						//initialize data to send as string
int sizeOfData = sizeof(dataToSendL2);				//number of letters in the sentence
char * pointerToData = dataToSendL2;                //pointer to the sentence
byte charInBin = *pointerToData;					//value of the pointer
uint8_t binData = 0; 								//binary integer for each letter in the data
uint8_t fullLetterBin = 255;						//leatter in binary after hamming (7+7 bit)
uint8_t bitMask = 128;								//multipy for get number by index
int binCreateIndex = 7;							    //index for binary number creation
bool tx_l2_request = false;							//flag to check if the transfer of layer 2 is already requested
bool rx_l2_falling = false;							//flag to check if the rx finished to recieve
uint8_t layer2Data = 0;							    //the data of layer 2
char recievedDataL2[100] = "";						//data in ascii
int pToRData = 0;									//index for the data array
//----------------------------Hamming tx function Variables-------------------------
uint8_t splitDataMSB = 0;                           //MSB(first 4 digits) digits in binary of each letter in the data
uint8_t splitDatalSB = 0;							//LSB(last 4 digits) digits in binary of each letter in the data
uint8_t hammedMSB = 0;								//MSB after hamming coding
uint8_t hammedLSB = 0;								//LSB after hamming coding
int readMSBflag = 0;								//flag for check if first half is sent
//----------------------------Hamming rx function Variables-------------------------
int firstWordFlag = 0;								//flag for check if first half is recieved
int letterKeep = 0;									//keep the full letter in binary
int data = 0;										//keep letter after decode	
//-----------------------------------CRC variables----------------------------------
//-----------------------------------Settings---------------------------------------
#define errorHamming 0b0000000                      //control the corrupt bits (1 is corrupt bit)

void setup() {
  	Serial.begin(9600);			 					//BAUD_RATE = 9600 
  	pinMode(txData, OUTPUT);   		 				//initialize Tx - analog pin
  	pinMode(rxData,INPUT);				 			//initialize Rx - analog pin
  	pinMode(txClk, OUTPUT);			 				//initialize clkIn 
  	pinMode(rxClk,INPUT);				 			//initialize clkOut  
}
void loop() {
  	layer2_tx(); 									//Layer 2 transfer function 
	layer2_rx(); 									//Layer 2 reciever function 
    layer1_tx(); 									//Layer 1 transfer function
	layer1_rx(); 									//Layer 1 reciever function      						    
}
void layer2_tx(){
  	if(tx_l1_busy)
      return;
  	if(binCreateIndex == -1)
    {
      Serial.println("------------------------L2 TX----------------------------");
      Hamming47_tx();
      binData = 0;
      binCreateIndex = 7;
      bitMask = 128;
      tx_l2_request = true;
      if(charInBin == '\0')
      {
        pointerToData = dataToSendL2;
        charInBin = *pointerToData;
      }
      else
      {
      	pointerToData++;      
      	charInBin = *pointerToData;
      }
      return;
    }
    if (bitMask & charInBin) 
      bitWrite(binData, binCreateIndex--, 1);
    else 
      bitWrite(binData, binCreateIndex--, 0);
    bitMask = bitMask >> 1;	
}
void layer2_rx(){
  	if(rx_l1_busy)
      rx_l2_falling = true;
  	if(rx_l2_falling == true && rx_l1_busy == false)
    {
      Serial.println("------------------------L2 RX----------------------------");
      rx_l2_falling = false;
      Hamming47_rx();
      if(!firstWordFlag)
      {
        letterKeep = data;
        firstWordFlag = true;
        return;
      }
      else
      {
        firstWordFlag= false;
        letterKeep = letterKeep << 4;
        letterKeep = letterKeep + data;
      }
      if(letterKeep != 0)
      {
          recievedDataL2[pToRData] = letterKeep;
          pToRData++;
          recievedDataL2[pToRData] = char('\0');
          Serial.println(recievedDataL2);
      }
      else
      {
          Serial.print("The Full Sentence is: ");
          Serial.println(recievedDataL2);
          pToRData = 0;
          strcpy(recievedDataL2, "");
      }
    }      
}
void layer1_tx()
{ 

  	if (tx_l2_request)
    {
      	tx_l2_request = false;
    	tx_l1_busy = true;  
      	dataToSendL1 = hammedMSB;
    }
    if(!tx_l1_busy)
      return;
    if(index >= SIZE)                               //check that max digits sent by SIZE
    {
      long randTime = random(200, 1000);			//wait random time between data
      if(!time(randTime))
        return;
      index = 0;
      if(!readMSBflag)
      {
        readMSBflag = true;
        dataToSendL1 = hammedLSB;
      }
      else
      {
      	tx_l1_busy = false;
        readMSBflag = false;
      }
      return;  
    }
    boolean leastSign = bitRead(dataToSendL1,index);//lsb bit
    long timer = millis(); 				 			//time since turn on
    if (timer - current >= BIT_TIME)           	    //Timer
    {
  		clkVal = !clkVal;  				 			//flip the 1->0 and 0->1
        if(clkVal)                                  //if bit is 1 send data
        {        				 			
            digitalWrite(7, LOW);                   //do not send clk
            if(leastSign) 				 			//check lsb
				digitalWrite(5, HIGH);   			//if 1 so datapin 5 is 5 volt
          	else
              	digitalWrite(5, LOW);    			//if 0 so datapin 5 is 0 volt
            index++;                     			//index of the data
        }
      	else
          	digitalWrite(7, HIGH);                  //if bit is 0 send clk      
      current = millis();                           //current time                        
    }
}
void layer1_rx()
{
  if (!digitalRead(8))							    //check if we are in "not read" mode
  {
    flagIfRead = 0;									//if does turn flag to zero
    rx_l1_busy = true;								//rx in llayer 1 is busy
  }
  
  else if(digitalRead(8) && flagIfRead == 0)		//check if we are in "read" mode and flag is zero
  {
    flagIfRead = 1;									//turn flag to 1- we already read
 	bitWrite(recievedData, count, digitalRead(6));  //turn the digits into one number
	count++;										//count size of the nuber
  }
  if (count == SIZE)                                //check that all bits recieved
  {
    Serial.println("------------------------L1 RX----------------------------");
   	Serial.println(recievedData,BIN);               //print the final data
    rx_l1_busy = false;	
    layer2Data = recievedData;
   	count = 0;                                      //reset counter for new data
    recievedData = 0;                               //reset data for new data
  }
}

bool time(long waitingTime)							//Time function for tx Function
{
  long timer = millis(); 				 			//time since arduino turned on
  if (timer - currentTX >= waitingTime)           	//waitingTime is passed
  {
    	currentTX = millis();						//save current time
    	return true;								//return the time is passed
  }
  else												//time didnt pass yet, return false
  		return false;									
}
void Hamming47_tx()
{
  Serial.println("------------------------HAMMING----------------------------");
  splitDataMSB = 0b11110000 & binData;
  splitDataMSB = splitDataMSB >> 4;
  splitDatalSB = 0b00001111 & binData;
  hammedMSB = createHammingNum(splitDataMSB);
  hammedLSB = createHammingNum(splitDatalSB);
  
  Serial.println("Hammed first 7 digits");
  Serial.println(hammedMSB,BIN);
  Serial.println("Hammed last 7 digits");
  Serial.println(hammedLSB,BIN);
}
int createHammingNum(uint8_t dataToHamming)
{
  int d1 = bitRead(dataToHamming, 3);
  int d2 = bitRead(dataToHamming, 2);
  int d3 = bitRead(dataToHamming, 1);
  int d4 = bitRead(dataToHamming, 0);
  int p1 = d1 ^ d2 ^ d4;
  int p2 = d1 ^ d3 ^ d4; 
  int p3 = d2 ^ d3 ^ d4;
  bitWrite(dataToHamming,6,p1);
  bitWrite(dataToHamming,5,p2);
  bitWrite(dataToHamming,4,d1);
  bitWrite(dataToHamming,3,p3);
  bitWrite(dataToHamming,2,d2);
  bitWrite(dataToHamming,1,d3);
  bitWrite(dataToHamming,0,d4);
  return dataToHamming;
}
void Hamming47_rx()
{
  Serial.println("------------------------DECODING----------------------------");
  Serial.print("The recieved data before noise is - ");
  Serial.println(layer2Data,BIN);
  Serial.print("The recieved data after noise is - ");
  layer2Data = layer2Data^errorHamming;
  Serial.println(layer2Data,BIN);
  int solution = 0;
  int p1 = bitRead(layer2Data, 6);
  int p2 = bitRead(layer2Data, 5); 
  int d1 = bitRead(layer2Data, 4);
  int p3 = bitRead(layer2Data, 3);
  int d2 = bitRead(layer2Data, 2);
  int d3 = bitRead(layer2Data, 1);
  int d4 = bitRead(layer2Data, 0);
  bitWrite(solution, 0, p1^d1^d2^d4);
  bitWrite(solution, 1, p2^d1^d3^d4);
  bitWrite(solution, 2, p3^d2^d3^d4);
  if(solution != 0)
  {
    Serial.print("The error is in bit number: ");
    Serial.println(solution, DEC);
    int flipBitIndex = bitRead(layer2Data, 7-solution);
    bitWrite(layer2Data,7-solution, !flipBitIndex);
  }
  bitWrite(data, 3, bitRead(layer2Data, 4));
  bitWrite(data, 2, bitRead(layer2Data, 2));
  bitWrite(data, 1, bitRead(layer2Data, 1));
  bitWrite(data, 0, bitRead(layer2Data, 0));
  Serial.print("The recieved data after decode is - ");
  Serial.println(data,BIN);  
}

