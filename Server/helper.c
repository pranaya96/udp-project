/*

  HELPER.C
  ========
  (c) Pranaya Adhikari, 2018
  

  Implementation of sockets helper functions.

  Many of these functions are adapted from, inspired by, or 
  otherwise shamelessly plagiarised from "Unix Network 
  Programming", W Richard Stevens (Prentice Hall).

*/

#include "helper.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <inttypes.h>

#define nul '\0'
#define TRUE 1


int readTypeZero(char* buffer, int currIndex, FILE *filePointer, char *targetType)
{
    int amount; // amount of numbers 
    amount = buffer[currIndex+1]; 
    
    char* numberBuffer;
    
    int translateAmount;
    translateAmount = amount;
    char amountArrayInAscii[4];
    uint8_t numValue;
    for(int i = 3; i > 0; --i) {
        numValue = translateAmount%10;
        numValue += 48;
        translateAmount = translateAmount / 10;
        amountArrayInAscii[i-1] = numValue;
    }
    amountArrayInAscii[3] = nul;
    //printf("The thing I like to print%s", amountArrayInAscii);

    uint16_t tempBuffer[amount]; // temporary Buffer to store individual number 
    uint16_t swapped[amount]; 
    printf("Amount:%d\n", amount);
    //printf("current index:%d\n", currIndex);
    
    for(int i = 0; i < amount; i++){
        memcpy(tempBuffer+i, (buffer+currIndex+2)+ i*2, 2);
        swapped[i] = (tempBuffer[i]>>8) | (tempBuffer[i]<<8); //change the byte order
        printf("%d\n", swapped[i]);
    }

    if (targetType[0] == '0' || targetType[0] == '2')
    {
        printToZeroType(filePointer, amount, tempBuffer);

    } else if (targetType[0] == '1' || targetType[0] == '3'){
        printToOneType(filePointer, amountArrayInAscii,amount, swapped);
    }
    else{
        printf("Target Type error");
        return -1;
    }

    return (currIndex+2+(amount*2));

}


int readTypeOne(char* buffer, int currIndex, FILE *filePointer, char* targetType)
{
    char amount[3]; //array to store amount followed by nul terminator
    memcpy(amount, buffer+currIndex+1, 3); // copy 3 bytes from primary buffer to amount array
    //amount[3] = nul ;//last index terminated by nul terminator
    printf("\n%s\n",amount);
    uint8_t numberAmount = atoi(amount); //change ascii to  8 bit integer
    printf("Amount:%d\n", numberAmount);
    //printf("------------------");
    char tempBuffer[6]; //5 bytes is needed at max for unsigned integers no more than 65535 plus nul terminator
    int currPosOfPointer = 4; // move pointer after reading the amount
    int i = 0;
    int bufferTracker=0;
    uint16_t converted[numberAmount];
    uint16_t swapped[numberAmount]; 
    //printf("current index:%d\n", currIndex);
    while (i < numberAmount){
        memcpy(tempBuffer+bufferTracker, buffer+currIndex+currPosOfPointer, 1);
        if(tempBuffer[bufferTracker]== 44){
            tempBuffer[bufferTracker] = nul;
            bufferTracker = 0;
            currPosOfPointer ++;
            printf("%d\n", atoi(tempBuffer));
            converted[i] = atoi(tempBuffer);
            swapped[i] = (converted[i]>>8) | (converted[i]<<8);
            //printf("I want to print xxx %" PRIu16 "\n",converted[i]);  
            i++;
            continue;

        }
        else if (tempBuffer[bufferTracker] == 0 || tempBuffer[bufferTracker] ==1){
            tempBuffer[bufferTracker] = nul;
            printf("%d\n", atoi(tempBuffer));
            converted[i] = atoi(tempBuffer);
            swapped[i] = (converted[i]>>8) | (converted[i]<<8);
            //printf("I want to print yyy%" PRIu16 "\n",converted[i]);
            i++;
            //printf("\nPointer Position:%d\n",  currIndex + currPosOfPointer);}
            if (targetType[0] == '0' || targetType[0] == '1')
            {
                printToOneType(filePointer,amount,numberAmount, converted);
        
            } else if (targetType[0] == '2' || targetType[0] == '3'){
                printToZeroType(filePointer, numberAmount, swapped);
            }
            else{
                printf("Target Type error");
                return -1;
            }
            
            return currIndex + currPosOfPointer;  
        }
        else{ 
            currPosOfPointer++;
            bufferTracker++;
            continue;
        }
    } 

    
    
    //printToZeroType(filePointer, numberAmount, converted);
    return currIndex + currPosOfPointer;//for the last number 

}



/*print in zero format*/
void printToZeroType(FILE *filePointer, uint8_t amount, uint16_t *numbersArray)
{
    uint8_t type = 0;
    //write the type
    fwrite(&type,sizeof(uint8_t),1, filePointer);
    //write the amount
    fwrite(&amount,sizeof(uint8_t),1, filePointer);
    //write the numbers

    for (int i=0; i<amount; i++) {
        fwrite(&numbersArray[i], sizeof(uint16_t), 1 ,filePointer);
    }
    
}


/*print in one format*/
void printToOneType(FILE *filePointer, char* arrayOfAmount,int amountCount, uint16_t *numbersArray){
    //write the type
    uint8_t type = 1;
    fwrite(&type,sizeof(uint8_t),1, filePointer);
    //write the amount
    for(int i=0; i < 3; i++){
        fprintf(filePointer,"%c",arrayOfAmount[i]);
    }
    //write the numbers
    for(int i=0; i < amountCount-1; i++){
        fprintf(filePointer,"%" PRIu16 ",",numbersArray[i]);
    }
    fprintf(filePointer,"%" PRIu16,numbersArray[amountCount-1]);    
}




