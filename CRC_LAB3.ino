// C code

#define BIT_TIME 10
#define SIZE 12
#define dataIn 5                		    // Set pin data out
#define dataOut 6                		    // Set pin data out
#define clockIn 7                		    // Set pin clkin
#define clockOut 8              		    // Set pin clkout
#define CRC_4 0b10011                       // x^4+x+1
#define lsbDecode 0b000000001111
#define msbDecode 0b111111110000

bool clkVal = false;
unsigned long DataSent;
int index = 0;
long current =0;
int flag = 0;
int count=0;
unsigned int DataIn=0;                    //initialize data to recieve
char *data2send = "Haim & Shahaf";
int i = 0; 
int temp=0; 
bool tx_l1_busy = false;							//flag to check if the transfer of layer 1 is already read
bool rx_l1_busy = false;							//flag to check if the reciever of layer 1 is already read
bool tx_l2_request = false;
int shiftL =7;
unsigned long currentTX = 0;
unsigned int recievedData=0;
bool flagIfRead = false;
unsigned int layer2Data = 0;
int encData = 0;
int parityNum = 0;
int tempRec = 0;
int shiftLRec =7;
bool rx_l2_falling = false;
bool rx_l2_busy = false;
char recievedAscii[100] = "";
int asciiIndex = 0;
int indexMSB = 11;
int indexMSBRec = 11;
#define errorCrc 0b000001000000                      

void setup()
{  
    Serial.begin(9600);       			 // Allows using prints to debug 
    pinMode(dataIn, INPUT);   		     // Set pin data out   
    pinMode(dataOut, OUTPUT);  			 // Set pin data in 
    pinMode(clockIn, INPUT);  			 // Set pin clkin
    pinMode(clockOut, OUTPUT);  			 // Set pin clkout
}



void loop()
{
      layer2_tx(); 					// Calls Hamming47_tx() or CRC4_tx()
      layer2_rx(); 					// Calls the Rx version of the above
      layer1_tx(); 					// Either uart_tx() or usart_tx() from Labs 1/2
      layer1_rx(); 					// Rx version of the above
}



void layer2_tx(){
      if(tx_l1_busy){
       		 return;  
      } 

      if(shiftL==7)
        {
            DataSent = data2send[i];             
            DataSent = DataSent<<4;
        	//Serial.println("*********");
            //Serial.print("binary before encoding: ");
            //Serial.println(data2send[i]);
        	//Serial.println(DataSent,BIN);
        	//Serial.println("*********");
        	if(DataSent == '\0')
              i=0;
        	else
              i++;              
            temp = DataSent;
            CRC4_tx();
            return;
        }

       else if(shiftL==-1){
         	//int x = DataSent;
         	//temp = temp >> 8;
         	DataSent = DataSent | temp;
         	//Serial.println(temp,BIN);
         	//Serial.println("*********");
            //Serial.print("After encoding: ");
            //Serial.println(DataSent,BIN);
            //Serial.println("*********");
            tx_l2_request =1; 				           // start sending 
         	shiftL =7;
         	indexMSB = 11;
            return;
        }
      else{
        //Serial.println(temp,BIN);	
        CRC4_tx();
      }
    } 				

void layer2_rx()
{
  if(rx_l1_busy)
      rx_l2_falling = true;
  if(rx_l2_falling == true && rx_l1_busy == false && !rx_l2_busy)
  {
    rx_l2_busy = true; 
    //parityNum = lsbDecode&layer2Data;
    encData = msbDecode&layer2Data;
    encData = encData>>4;
    tempRec = layer2Data^errorCrc;
    Serial.println(layer2Data, BIN);
    shiftLRec = 7;
    rx_l2_falling = false;
  }
  	if(rx_l2_busy)
    {
      if(shiftLRec==-1)
      {
        if(tempRec == 0)
        {
           if(encData == 0)
           {
             Serial.println("The full sentence is: ");
             Serial.println(recievedAscii);
             strcpy(recievedAscii, "");
             asciiIndex=0;
             indexMSBRec = 11;
           }
           //encData = encData >> 4;
           Serial.println("The letter is correct: ");
           Serial.println(encData,BIN);
           Serial.println(char(encData));
           recievedAscii[asciiIndex] = char(encData);
           asciiIndex++;
           indexMSBRec = 11;
           rx_l2_busy = false;
        }
        else
        {
           Serial.println(tempRec, BIN);
           Serial.println("The data is corrupt");
           rx_l2_busy = false;

        }
      }
      else
      {
          CRC4_rx();
      }
    }
}

void layer1_tx(){ 
     	if (tx_l2_request)
    {
        //Serial.println(DataSent);  
      	tx_l2_request = false;
    	tx_l1_busy = true;  
    }
    if(!tx_l1_busy)
      return;
    if(index >= SIZE)                               //check that max digits sent by SIZE
    {
      long randTime = random(200, 1000);			//wait random time between data
      if(!time(randTime))
        return;
          index = 0;
          tx_l1_busy=0;
          return;
    }

	byte lastsign = bitRead(DataSent,index); 	
	long timer = millis();                               
    if(timer - current >= BIT_TIME/2){         // Data is sent when the clock channel rises
      clkVal=!clkVal;                          // flip 1->0 or 0->1
      if(clkVal){
        
       digitalWrite(clockOut,LOW);
        	if(lastsign)
          		digitalWrite(dataOut, HIGH);   // send 1 
          
          	else
          		digitalWrite(dataOut, LOW);     // send 0

        index++;}
          
      else
      	  digitalWrite(clockOut,HIGH);
      current=millis();
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
   	//Serial.println(recievedData,BIN);               //print the final data
    rx_l1_busy = false;	
    layer2Data = recievedData;
   	count = 0;                                      //reset counter for new data
    recievedData = 0;                               //reset data for new data
  }
}


void CRC4_tx(){
  	if(shiftL!=-1)					        //10010000000 = H
    {  									    //100110000000 
      int crc = CRC_4<<shiftL; 				//110000000000
      if(crc > temp && shiftL == 7)
      {
         indexMSB--;
         shiftL--;
         //Serial.println("Moved"); 
         //Serial.println(temp,BIN);
      	 return;
        
      }
      if(bitRead(temp, indexMSB) == 1){
         //Serial.println(temp,BIN); 
         //Serial.println(crc,BIN); 
         temp = temp^crc;
         shiftL--;
         indexMSB--;
         //Serial.println("--------------------");       
         //Serial.println(temp,BIN);
         //Serial.println("");  
    }
      else
      {
         //Serial.println(temp,BIN); 
         //Serial.println(0b00000,BIN); 
         temp = temp^0b0;
         shiftL--;
         indexMSB--;
         //Serial.println("--------------------");       
         //Serial.println(temp,BIN);
         //Serial.println(""); 
      }
  }
}  
void CRC4_rx(){
  
  if(shiftLRec!=-1)					        //10010000000 = H
    {  									    //100110000000 
      int crcRec = CRC_4<<shiftLRec; 				//110000000000
      if(crcRec > tempRec && shiftLRec == 7)
      {
         indexMSBRec--;
         shiftLRec--;
         //Serial.println(tempRec,BIN);    
      	 return;
      }
    if(bitRead(tempRec, indexMSBRec) == 1){
         tempRec = tempRec^crcRec;
         shiftLRec--;
         indexMSBRec--;
      //Serial.println(tempRec,BIN);   
    }
    else
      {
           
         tempRec = tempRec^0b0;
         shiftLRec--;
         indexMSBRec--;
       //Serial.println(tempRec,BIN);    
      }
  }
}
