HANDLE hOpen=-1;
#define LENGTH 20 //256
int iRxData=0;
bool flag=false;
int count=0;
void Timer5OnExecute(tWidget *pWidget)
{
   //--- The buffer to storage the data, it's size is User-defined value ---
   static char recv_str[LENGTH];
   int res=0,ret=0;

   //--- If handle is invalid, return ---
   if(hOpen < 0)  return;

   //--- If no data received, return ---
   if(uart_GetRxDataCount(hOpen)==0) return;

   //--- Check whether the data has been transferred completly   ---
   if (iRxData != uart_GetRxDataCount(hOpen))
   {
    iRxData = uart_GetRxDataCount(hOpen);
    return;
   }

   //--- Make sure the message don't overflow the buffer ---
   iRxData = (iRxData<LENGTH)?iRxData:LENGTH;

   //--- Receive the data from COM port ---
   res = uart_BinRecv(hOpen, recv_str,iRxData);
   recv_str[iRxData]=0;

   //--- Purge Rx Buffer ---
   uart_Purge(hOpen, 0, 1);

   //--- Process all recevied data ---
   if (res)
   {
      hmi_Beep();

      //--- echo the received message ---
      LabelTextSet(&Label4, recv_str);
      ret = uart_BinSend(hOpen, recv_str, iRxData);
      iRxData = 0;

      if (ret)  LabelTextSet(&Label9, "Echo OK");
      else      LabelTextSet(&Label9, "Echo Error");
   }
}

void BitButton10OnClick(tWidget *pWidget)
{

   //--- Close the existing handle ---
  if(hOpen>=0)
  {
     uart_Close(hOpen);
     hOpen = -1;
  }

  //--- Establish a new handle ---
  if(hOpen<0)
  {
     hOpen = uart_Open("COM1,9600,N,8,1");
  }

  //---  If success, display current COM Port settings on screen ---
  if(hOpen>=0)
  {
  LabelTextSet(&Label8, "COM1,9600,N,8,1");
  uart_SetTimeOut(hOpen, 300);
  }

  /*
  if(hOpen>=0)
      {
          BOOL ret;
          unsigned char *Buffer = {'AA'};//, '55', '04', '01', '03', '19', '20'};
          char* buf;
          ByteToChar(Buffer, buf, 1);

          ret = uart_Send(hOpen, buf);
      }
    */
}

/*
void ByteToChar(unsigned char* bytes, char* chars, unsigned int count)
{
    for(unsigned int i = 0; i < count; i++)
    	 chars[i] = (char)bytes[i];
}

void stringtobyte()
{
    //string s;
    int nBytes = 0;
  //unsigned char Buffer[1000]="0xAA, 0x55, 0x04, 0x01, 0x03, 0x19, 0x20";//
  unsigned char *Buffer = "AA, 55, 04, 01, 03, 19, 20";
   char* s;
  ByteToChar(Buffer, s, 7);


  char *EndPtr;

  int str_len = strlen(Buffer);
   const char* chString;
  //itoa(str_len, chString, 10);
   //sprintf(chString,"%d",str_len);
   //printf("hi");
   //sprintf(c, "%d",str_len);
   usprintf(chString, "%d", 10);
  LabelTextSet(&Label9, chString);
  //Buffer = "AA 55 04 01 03 19 20";

}
 */

void BitButton11OnClick(tWidget *pWidget)
{
  if(hOpen>=0)
  {
     uart_Close(hOpen);
     hOpen = -1;
     LabelTextSet(&Label8, "Press 'Start' to Begin");
     LabelTextSet(&Label4, "");
     LabelTextSet(&Label9, "");
  }
}


void BitButton13OnClick(tWidget *pWidget)
{
   /*
   if(hOpen>=0)
      {
          BOOL ret;
          unsigned char *Buffer = "AA, 55, 04, 01, 03, 19, 20";
          char* buf;
          ByteToChar(Buffer, buf, 7);

          ret = uart_Send(hOpen, buf);
      }
      */
}


void BitButton12OnClick(tWidget *pWidget)
{
      //--- Close the existing handle ---
  if(hOpen>=0)
  {
     uart_Close(hOpen);
     hOpen = -1;
  }

  //--- Establish a new handle ---
  if(hOpen<0)
  {
     hOpen = uart_Open("COM1,9600,N,8,1");
  }

  //---  If success, display current COM Port settings on screen ---
  if(hOpen>=0)
  {
  LabelTextSet(&Label8, "COM1,9600,N,8,1");
  uart_SetTimeOut(hOpen, 300);
  }


     if(hOpen>=0)
      {
          BOOL ret;

          char Buffer[1000];
          //char buf[]="AA 55 04 01 03 19 20";
          //char *test = strtok(buf, " ");
          unsigned char buf[]={0xAA, 0x55, 0x04, 0x01, 0x03, 0x19, 0x20};

          //int str_len = strlen(buf);
          char *ends=(char*)buf;
          ends[7]='\0';

          /*
          for(int i=0;i<str_len;i++)
          {
             if(buf[i]!=' ')
             {
                 Buffer[i] = strtol(buf[i], &ends, 16);
             }
          }
          Buffer[str_len]="";
         */
         //LabelTextSet(&Label8, (char*)10);
          //for(int nBytes=0;nBytes<str_len;nBytes++)
          //Buffer[nBytes] = strtol(test[nBytes], &ends, 16);
          //char* buf="p";
          //ByteToChar(Buffer, buf, 7);

          //char *s = "70", *ends;
          //char c = (char)strtol(s, &ends, 16);

          ret = uart_Send(hOpen, ends);
      }
}


void BitButton14OnClick(tWidget *pWidget)
{
     /*
     if(flag==false)
     {
        flag=true;
        //BitButtonTextSet
     }
     else
     {
        flag=false;
     }
     */
}


void TextPushButton15OnClick(tWidget *pWidget)
{
      count=0;
      if(flag==false)
     {
        flag=true;
        //BitButtonTextSet
        TextButtonTextSet((tTextButton*)pWidget, "stop");

     }
     else
     {
        flag=false;
         TextButtonTextSet((tTextButton*)pWidget, "start");
     }


}


void Timer13OnExecute(tWidget *pWidget)
{
      if(flag==true)
      {
         static char str[16];
         count++;
         usprintf(str, " %d sec", count);
         LabelTextSet(&Label8, str);
      }
}

