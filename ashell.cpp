#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
using namespace std;


void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    //char *name;
    
    // Make sure stdin is a terminal. 
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later. 
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes. 
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO. 
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}


void clearSTDOUT(int &sizeOfString, char *currentLine ){
    string deleteString("\b \b");
    
    for(int i = 0; i < sizeOfString ; i++){
        write(STDOUT_FILENO, deleteString.c_str(), 3);
        currentLine[i] = 0;
    }
    sizeOfString = 0;
}
//FUNCTION THAT CHANGES CURRENTLINE AND UPDATES ITS CURRENTLINESIZE

// template <class myVectorIterator>
// void updateCurrentLine(const char *currentLine, myVectorIterator it2){
//     currentLine = it2->c_str();
//     //it2->c_str()
//    // cout << "   St ring is :" << str << endl;
// }

void printCurrentDir(){ //http://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
    //string space("")
    char buffer[1024];
    char *currentDir = getcwd(buffer, sizeof(buffer));
    
    if(strlen(currentDir) < 16){
        for(int i = 0; i < strlen(currentDir); i++){
            write(STDOUT_FILENO, &currentDir[i], 1);
        }
    }

    else{//total path is more than 16 chars
//stackoverflow.com/questions/32822988/get-the-last-token-of-a-string-in-c
        const char delimiter[2] = "/";
        char *token, *last;
        last = token = strtok(currentDir, "/");

    
        for(; (token = strtok(NULL, "/")) != NULL; last = token);

        write(STDOUT_FILENO, "/.../", 5);

        for(int i = 0; i < strlen(last); i++)
            write(STDOUT_FILENO, &last[i], 1);
    }

    write(STDOUT_FILENO, "% ", 2);
}

int main(int argc, char *argv[]){
    struct termios SavedTermAttributes;
    char RXChar;
    char currentLine[10000];
    int currentLineSize = 0;
    vector<string> myVector;
    vector<string>::iterator it2; //vector iterator
    int numPrevCommands = 0;
    string deleteString("\b \b");
    string soundString("\a");
    int historyCounter = 0;
    bool upArrowOnce = false;
    bool downArrowOnce = false;
    
//    string

    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);

    printCurrentDir();
    
    while(1){
        read(STDIN_FILENO, &RXChar, 1);

        //IF IT IS A PRINTABLE CHARACTER
        if(isprint(RXChar)){ // || RXChar == 0x0A){
            write(STDOUT_FILENO, &RXChar, 1);
            currentLine[currentLineSize++] = RXChar;
        }//WRITE IT OUT TO THE SCREEN, THEN ADD THAT CHARACTER INTO A LIST

        //IF CTRL+D IS TAKEN AS AN INPUT, THEN EXIT
        if(0x04 == RXChar){
            break;
        }
        //IF YOU PRESS BACKSPACE
        else if(0x7F == RXChar){
            write(STDOUT_FILENO, deleteString.c_str(), 3);
            if(currentLineSize >= 1)
                currentLineSize--;
            currentLine[currentLineSize] = 0;
        }
        //OTHERWISE
        else{

            //IF ' '
            if(0x1b == RXChar){

                read(STDIN_FILENO, &RXChar, 1);

                //IF '['
                if(RXChar == 0x5b){
                    read(STDIN_FILENO, &RXChar, 1);

                    //IF UP ARROW , 'A'
                    if(RXChar == 0x41){                     
                        downArrowOnce = false;

                        //IF VECTOR IS EMPTY, THEN BEEP
                        if(myVector.empty()){
                            write(STDOUT_FILENO, soundString.c_str(),1);
                        }
                        else{

                            //IF YOU'RE NOT AT THE END OF THE VECTOR
                            if( (it2 != myVector.end() - 1) ){
                                //cout << "BEFORE CURRENT LINE IS: " << currentLine << endl;
                                //cout << " our it2S tring is :" << it2->c_str() << endl;
                                clearSTDOUT(currentLineSize, currentLine);
                                //cout << "current line size is: " << currentLineSize << endl;
                               // cout << "CURRENT LINE IS: " << currentLine << endl;

                                write(STDOUT_FILENO, it2->c_str(), it2->size());


                                currentLineSize = it2->size();
                                strcpy(currentLine, it2->c_str());


                                ++it2;
                            }//THEN GO TO THE NEXT
                            //ELSE IF YOU'VE DISPLAYED THE END ALREADY, THEN BEEP
                            else if(upArrowOnce == true){
                                write(STDOUT_FILENO, soundString.c_str(),1);
                            }
                            //ELSE IF YOU'RE AT THE END, THEN DISPLAY IT
                            else{
                                clearSTDOUT(currentLineSize, currentLine);
                                write(STDOUT_FILENO, it2->c_str(), it2->size());


                                currentLineSize = it2->size();
                                strcpy(currentLine, it2->c_str());
                                //cout << "current line IS" << currentLine << endl;
                                upArrowOnce = true;
                            }//if this is the end
                        }
                    }
                    //IF DOWN ARROW 'B'
                    else if(RXChar == 0x42){ // 'B' down arrow
                        upArrowOnce = false;

                        if(myVector.empty()){
                            write(STDOUT_FILENO, soundString.c_str(),1);
                        }
                        else{
                            clearSTDOUT(currentLineSize, currentLine);
                            //IF ITERATOR NOT AT BEGINNING OF VECTOR
                            if(it2 != myVector.begin()){
                                it2--;
                                write(STDOUT_FILENO, it2->c_str(), it2->size());

                                currentLineSize = it2->size();
                                strcpy(currentLine, it2->c_str());

                            }
                            else if(downArrowOnce == true){
                                write(STDOUT_FILENO, soundString.c_str(),1);
                            }
                            else{
                                downArrowOnce = true;
                                
                            }//else write onto screen

                        //printf("It was a down arrow\n");
                        }
                    }
                    //OTHERWISE ITS SOMETHING ELSE
                    else
                        printf("False alarm\n");

                }
            }

            //IF WE PRESSED ENTER AND THERE IS SOMETHING IN CURRENT LINE
            if(0x0a == RXChar && currentLineSize != 1){
                //cout << "went into here" << endl;
                historyCounter++;


                //cout << "current line size is " << currentLineSize << endl;
                //IF WE HAVE MORE THAN 10 IN OUR HISTORY
                if(historyCounter > 10){
                    myVector.pop_back();
                    historyCounter--;
                }//DELETE ONE IN OUR HISTORY, AND DECREMENT HISTORY COUNTER

                //REMOVES NEWLINE AT THE END
                currentLine[currentLineSize] = 0;
                currentLineSize = 0;

                //INSERTS CURRENTLINE INTO VECTOR
                string str(currentLine);
                myVector.insert(myVector.begin(), str);

                it2 = myVector.begin();
                numPrevCommands++;

                write(STDOUT_FILENO, "\n", 1);
                printCurrentDir();


            }
            //ELSE IF WE PRESS ENTER WITH EMPTY LINE
            else if(0x0a == RXChar ){
                currentLine[currentLineSize-1] = 0;
                currentLineSize = 0;
            }
        }
    }
    
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    return 0;
}
