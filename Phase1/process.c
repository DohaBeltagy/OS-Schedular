#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int argc, char * argv[])
{
    initClk();
     
    // Convert the runtime argument from string to integer
    int runtime=atoi(argv[1]);
    remainingtime=runtime;
    // Now you can use the 'runtime' variable in your program
    printf("Runtime received: %d\n", remainingtime);



    //TODO it needs to get the remaining time from somewhere
    //remainingtime = ??;
    while (remainingtime > 0)
    {
        printf("alooo from process \n");
        sleep(1);
        // remainingtime = ??;
    }
    
    destroyClk(false);
    
    return 0;
}
