#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
//#include <windows.h>
#include "rs232.h"
#include "serial.h"

#define bdrate 115200               /* 115200 baud */

void SendCommands (char *buffer );

struct textfile
{

    int Num1;
    int Num2;
    int Num3;
    
};   

int scale(int user_input)
{
        int scaledValue = user_input/18;
        return scaledValue;
};


int main()
{
    //opening file1 and initilising array
    char *TextFileArray = NULL;
    int size = 0;

    struct textfile FileArray[1027];
    FILE *file;
    int i;


    file = fopen ("SingleStrokeFont.txt", "r");
    if (file == NULL)
    {
        printf("Error, cannot open file.\n");
        return 1;

    }

    for ( i = 0 ; i<10 ; i++)
    {
        fscanf(file,"%d %d %d ", &FileArray[i].Num1, &FileArray[i].Num2, &FileArray[i].Num3);

    }
    fclose(file);
    
    //scale
    int user_input;
    printf("Give a value between 4 and 10/n");
    scanf("%d", &user_input);
    int scaledValue = scale(user_input);
    
    //opening file 2 from user input
    FILE *file2;
    int j;
    char textfileInput[100];
    printf("What is the name of the text file you want to open?/n");
    scanf("%s" , &textfileInput);

    file2 = fopen(textfileInput, "r");

    if (file2 == NULL)
    {
        printf("Error, cannot open file.\n");
        return 1;

    }
    while ((j = fgetc(file2)) != EOF) 
    {
        // Ignores non alphabet characters
        if (j == '\n' || j == '\r') continue;

        // changes size of array
        char *temp = realloc(TextFileArray, (size + 1) * sizeof(char));
        if (temp == NULL) 
        {
            printf("Error allocating memory");
            TextFileArray = NULL;
            fclose(file2);
            return 1;
        }
        TextFileArray = temp;

        TextFileArray[size] = j;
        size++;
        
    }
    fclose(file2);


   TextFileArray = NULL;

    //char mode[]= {'8','N','1',0};
    char buffer[100];

    // If we cannot open the port then give up immediately
    if ( CanRS232PortBeOpened() == -1 )
    {
        printf ("\nUnable to open the COM port (specified in serial.h) ");
        exit (0);
    }

    // Time to wake up the robot
    printf ("\nAbout to wake up the robot\n");

    // We do this by sending a new-line
    sprintf (buffer, "\n");
     // printf ("Buffer to send: %s", buffer); // For diagnostic purposes only, normally comment out
    PrintBuffer (&buffer[0]);
    Sleep(100);

    // This is a special case - we wait  until we see a dollar ($)
    WaitForDollar();

    printf ("\nThe robot is now ready to draw\n");

        //These commands get the robot into 'ready to draw mode' and need to be sent before any writing commands
    sprintf (buffer, "G1 X0 Y0 F1000\n");
    SendCommands(buffer);
    sprintf (buffer, "M3\n");
    SendCommands(buffer);
    sprintf (buffer, "S0\n");
    SendCommands(buffer);

    int offset = 0;
    int spacing = 5;
    // These are sample commands to draw out some information - these are the ones you will be generating.
 for (int k = 0; k < size; k++) 
 {
    int asciiValue = (int)TextFileArray[k];
    int xValue = asciiValue + offset;

    if (k > 0)  
    {
        xValue += spacing;
    }


    sprintf(buffer, "G01 X%d Y%d ; Character: %c (ASCII: %d)\n", asciiValue, asciiValue, TextFileArray[k], asciiValue);
    SendCommands(buffer);
    offset += spacing;
 }
    /*sprintf (buffer, "G0 X-13.41849 Y0.000\n");
    SendCommands(buffer);
    sprintf (buffer, "S1000\n");
    SendCommands(buffer);
    sprintf (buffer, "G1 X-13.41849 Y-4.28041\n");
    SendCommands(buffer);
    sprintf (buffer, "G1 X-13.41849 Y0.0000\n");
    SendCommands(buffer);
    sprintf (buffer, "G1 X-13.41089 Y4.28041\n");
    SendCommands(buffer);
    sprintf (buffer, "S0\n");
    SendCommands(buffer);
    sprintf (buffer, "G0 X-7.17524 Y0\n");
    SendCommands(buffer);
    sprintf (buffer, "S1000\n");
    SendCommands(buffer);
    sprintf (buffer, "G0 X0 Y0\n");
    SendCommands(buffer);*/

    // Before we exit the program we need to close the COM port
    CloseRS232Port();
    printf("Com port now closed\n");

    return (0);
}

// Send the data to the robot - note in 'PC' mode you need to hit space twice
// as the dummy 'WaitForReply' has a getch() within the function.
void SendCommands (char *buffer )
{
    // printf ("Buffer to send: %s", buffer); // For diagnostic purposes only, normally comment out
    PrintBuffer (&buffer[0]);
    WaitForReply();
    Sleep(100); // Can omit this when using the writing robot but has minimal effect
    // getch(); // Omit this once basic testing with emulator has taken place
}

