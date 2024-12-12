#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
//#include <windows.h>
#include "rs232.h"
#include "serial.h"

#define bdrate 115200 
#define FILE1_ARRAY_SIZE 1027              /* 115200 baud */

void SendCommands (char *buffer );
//creating structure
struct textfile
{

    int Num1;
    int Num2;
    int Num3;
    
};   
//creating scale function
float scale(float user_input)
{
        float scaledValue = user_input/18;
        return(scaledValue);
}


int main()
{
    //opening file1 and initilising array
    char *TextFileArray = NULL;
    int size = 0;

    struct textfile File1Array[FILE1_ARRAY_SIZE];
    FILE *file1;
    int i=0;


    file1 = fopen ("SingleStrokeFont.txt", "r");
    if (file1 == NULL)
    {
        printf("Error, cannot open file.\n");
        return 1;

    }
   
    char line[256];

    while (fgets(line, sizeof(line), file1)) 
    {
        // Parse each line and store into the struct array
        if (sscanf(line, "%d %d %d", &File1Array[i].Num1, &File1Array[i].Num2, &File1Array[i].Num3) == 3) {
            i++; // Move to the next struct
        } else {
            fprintf(stderr, "Invalid line format: %s", line);
        }
        printf("%s",line);
    }
    fclose(file1);
    //scale
    int user_input;

     do {
        printf("\nGive a value between 4 and 10:\n");
        if (scanf("%d", &user_input) != 1) {
            // Handle invalid input (non-integer)
            printf("Invalid input. Please enter an integer.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        // Check if input is out of range
        if (user_input < 4 || user_input > 10) {
            printf("Input value not in the range. Try again.\n");
        }
    } while (user_input < 4 || user_input > 10);

    // Proceed with valid input
    printf("You entered a valid value: %d\n", user_input);
    float scaledValue = scale(user_input); // Assuming `scale` is a function

    
    //opening file 2 from user input
    FILE *file2;
    int j;
    char textfileInput[100];
    printf("What is the name of the text file you want to open?\n");
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
            free(TextFileArray); // 
            fclose(file2);
            return 1;
        }
        TextFileArray = temp;
        

        TextFileArray[size] = j;
        size++;
        printf("\nvalue read from file is %d\n",TextFileArray[size - 1]);
    }
    fclose(file2);


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

    float xOffset = 0;  // Horizontal offset
    float yOffset = 0;  // Vertical offset
    float lineSpacing = 10;
    float charSpacing = 5;  // Distance between lines
    float maxLineWidth = 100;
    
    // These are sample commands to draw out some information - these are the ones you will be generating.
    for (int k = 0; k < size; k++) 
    {
        float asciiValue = (int)TextFileArray[k];
        if (asciiValue == '\n' || (xOffset + charSpacing * scaledValue) >= maxLineWidth)
        {
            xOffset = 0;  // Reset horizontal offset
            yOffset -= lineSpacing;  // Increase vertical offset
            if (TextFileArray[k] == '\n') 
            {
                continue;
            }
        }
                // Loop through each entry in the font data array
                for (int fontline = 0; fontline < FILE1_ARRAY_SIZE; fontline++)
                {
                    if (File1Array[fontline].Num1 == 999 && File1Array[fontline].Num2 == asciiValue) 
                    {
                        int numLines = File1Array[fontline].Num3; 
                        fontline++; // Move to the first line of G-code for this character
                        // Output each G-code line for the character
                        for (int line2= 0; line2 < numLines && fontline < FILE1_ARRAY_SIZE; line2++, fontline++)
                        {
                            float xValue = File1Array[fontline].Num1 * scaledValue + xOffset;
                            float yValue = File1Array[fontline].Num2 * scaledValue + yOffset;
                            const char* pen = File1Array[fontline].Num3 == 1 ? "S1000" : "S0";
                            int gCode = File1Array[fontline].Num3 == 1 ? 1 : 0;
                            SendCommands(pen);

                            sprintf(buffer, "G%d X%.2f Y%.2f \n",gCode, xValue, yValue);
                            //printf(pen);
                            //SendCommands(pen);
                            //printf("%s",buffer);
                            SendCommands(buffer);
                        }
                        xOffset += charSpacing  ;
                        break;
                    }
                //break;
                }
    }

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
    //WaitForReply();
    Sleep(100); // Can omit this when using the writing robot but has minimal effect
    // getch(); // Omit this once basic testing with emulator has taken place
}

