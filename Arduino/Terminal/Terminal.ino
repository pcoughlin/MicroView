/*
     Teminal Demo for MicroView
*/

#include <MicroView.h>
#include <elapsedMillis.h>
#include <SoftwareSerial.h>

#define BLINK 333    // milliseconds
#define SPACE 32

//SoftwareSerial mySerial(2, 3); // RX, TX
HardwareSerial& mySerial = Serial;

// Global variables
int columns = 0;
int rows = 0;
int current_column = 0;
int current_row = 0;
int current_blink = 0;
elapsedMillis cursorBlink = 0;

// The virtual screen, assume tiny tiny font  (note the MicroView/Uno only has 3072 bytes SRAM!)
char screen[20][20] = {0}; // rows, columns

char char_under_cursor = SPACE;

void setup()
{
  uView.begin();	// begin of MicroView  
  mySerial.begin(2400);
  clearscreen(); 
  cursorBlink = 0;
  
 // uView.println("READY.");
  mySerial.println("READY.");  
}

void loop() 
{
    // Blink Cursor
    if (cursorBlink > BLINK)
    {
       cursorBlink = 0;
       
       if (current_blink == 0)
       {
         current_blink = 1;  
         char_under_cursor = getChar(current_row, current_column);
         setChar(current_row, current_column, 218); 
       }
       else // 1
       {
         current_blink = 0;
         restoreCharUnderCursor();
       }
    }  
 
    // Process Input
    if (mySerial.available() > 0)
    {
       char c=mySerial.read();
       
       // Special characters (that are never printed)
       switch (c)
       {
         case 27: // Escape
                  doEscapeSequence();
                  return;
                  
         case 1:  // CTRL-A, clear    
                  clearscreen();
                  return;
                 
         case 13: // Enter      
                  restoreCharUnderCursor();
                  nextLine();
                  return;
                  
         case 127: // Backspace
         case 8:   // Shift-Backspace         
                   if (current_column > 0) 
                   {
                     restoreCharUnderCursor();
                     current_column--; 
                     char_under_cursor = SPACE;
                     restoreCharUnderCursor();
                   }
                   return;
                  
       }
       
       // All others, print character
      
       setChar(current_row, current_column, c);
       current_column++;   
       
       if (current_column >= columns)
       {
          nextLine();
       }

       // Echo
       mySerial.print(c); 
    }     
    
    // Update
    uView.display();
}


void clearscreen()
{
  uView.clear(ALL);	// erase hardware memory inside the OLED controller
  uView.clear(PAGE);	// erase the memory buffer, when next uView.display() is called, the OLED will be cleared.
  setupFont(0);
  
  uView.setCursor(0,0);
  uView.display();

  current_column = 0;
  current_row = 0;
  
  for (int r=0; r<rows; r++)
  {
      for (int c=0; c<columns; c++)
      {
         screen[r][c] = SPACE;
      }
  }  
  char_under_cursor = SPACE;
}
  
void setupFont(int fontnum)
{
   uView.setFontType(fontnum);  
   rows = floor(uView.getLCDHeight() / uView.getFontHeight());
   columns = floor(uView.getLCDWidth() / uView.getFontWidth());  
   
   // TODO: Investigate why this is needed
   rows--;
   columns--;
}

void setChar(int row, int column, char c)
{
  // Crude Range Checking
  if (row<0) return;
  if (row>=rows) return;
  if (column<0) return;
  if (column>=columns) return;
  
  screen[row][column] = c;
  
  int realr = row*(uView.getFontHeight()+1);
  int realc = column*(uView.getFontWidth()+1);
  uView.setCursor(realc,realr);
  uView.print(c);
}

byte getChar(int row, int column)
{
  // TODO: Range Checking
  
  return screen[row][column];
}

void nextLine()
{
   current_column = 0;
   current_row++;
   checkScroll();
   
   mySerial.println();
}

void checkScroll()
{
   if (current_row < rows)  // No action needed
   {
     return;
   }
   
  for (int c=0; c<columns; c++)
  {
      for (int r=0; r<rows; r++)
      {
         setChar(r,c,getChar(r+1,c));
      }
  }  
  current_row = rows-1;
  
  uView.display();
}

void restoreCharUnderCursor()
{
  setChar(current_row, current_column, char_under_cursor); 
}

void doEscapeSequence()
{
  // Assumption that the sequence is always received in full.  No timeout handling, yet.
  char c1 = getByte();
  char c2 = getByte();
  
  // We only handle cursor movement.  Ignore all others.
  if (c1 != 0x5B) return;
  
  switch (c2)
  {
    case 65: // Up
             if (current_row > 0) 
             {
               restoreCharUnderCursor(); 
               current_row--; 
               char_under_cursor = getChar(current_row, current_column);  
             }
             break;
             
    case 66: // Down
             restoreCharUnderCursor();
             current_row++;  
             checkScroll();
             char_under_cursor = getChar(current_row, current_column);
             break;
             
    case 67: // Right
             if (current_column < (columns-1))
             {
                restoreCharUnderCursor();
                current_column++;  
                char_under_cursor = getChar(current_row, current_column);
             }
             break;
             
    case 68: // Left
             if (current_column > 0) 
             {
               restoreCharUnderCursor();
               current_column--; 
               char_under_cursor = getChar(current_row, current_column);
             }
             break;
             
    default: // Ignore all others
             return;
  }
  uView.display();
}
      
       
byte getByte()
{  
  // Wait for a byte
  while(!mySerial.available()) 
  {
     ; 
  }
 
  byte inByte = mySerial.read();
  return inByte;
}

         

     /* Font Tests
     clearscreen(); 
     uView.print((int)i);
     uView.print("=");
     uView.println(i);
     uView.display();
     i++;
     */
       
       
