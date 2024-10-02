#include "os3k.h"

APPLET_HEADER_BEGIN
    APPLET_ID(0xA1C0)
    APPLET_NAME("Ballistics Calculator")
    APPLET_INFO("Calculate fast things flying through the air")
    APPLET_VERSION(BETAWISE_VERSION_MAJOR, BETAWISE_VERSION_MINOR, BETAWISE_VERSION_REVISION)
    APPLET_LANGUAGE_EN_US
APPLET_HEADER_END

// Declaration of global variables at the file scope
// G7 Coefficient * 1000
int g_G7Coefficient = 0;  
int g_MuzzleVelocity = 0; 
char g_InputBuffer[128];
int g_CurrentInputField = 0;
char debugOutput[64]; 

#define DEBUG_CHARS false

// Parse integers only for fixed-point input
 // Assumes input is in the format like "690" for 0.69
int parse_fixed(const char* str) {
    int result = 0;
    while (*str != '\0') {
        if (*str >= '0' && *str <= '9') {
            result = result * 10 + (*str - '0');
        }
        str++;
    }
    return result; 
}

bool is_alphanumeric(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int multiply(int a, int b) {
    int result;
    if (a == 0 || b == 0) {
        PutString("Warning: Multiplication operand is zero\n");
        return 0;
    }
    __asm__ volatile (
        "move.l %1, %%d0\n\t"        // Load 'a' into d0
        "move.l %2, %%d1\n\t"        // Load 'b' into d1
        "muls.w %%d1, %%d0\n\t"      // d0 = d0 * d1
        "move.l %%d0, %0\n\t"        // Store result from d0 to 'result'
        : "=r" (result)              // Output
        : "r" (a), "r" (b)           // Input
        : "d0", "d1", "cc"           // Clobbered register
    );
    return result;
}

int divide(int a, int b) {
    int result;
    if (b == 0) {
        sprintf(debugOutput, "Division by zero error: a=%d, b=%d\n", a, b);
        PutString(debugOutput);  // Display error on device, if possible
        return -1; 
    }
    __asm__(
        "move.l %1, %%d0\n\t"        // Load 'a' into d0
        "move.l %2, %%d1\n\t"        // Load 'b' into d1
        "divs.w %%d1, %%d0\n\t"      // Perform signed division: d0 = a / b
        "move.l %%d0, %0\n\t"        // Store the result from d0 to 'result'
        : "=r" (result)              // Output
        : "r" (a), "r" (b)           // Inputs
        : "d0", "d1"                 // Clobbered registers
    );
    if (__builtin_expect(result < 0, 0)) {
        char buffer[64];
        //sprintf(buffer, "Division error: a=%d, b=%d\n", a, b);
        sprintf(debugOutput, "Division result: %d / %d = %d\n", a, b, result);
        PutString(buffer);
    }
    return result;
}
// Manual implementation of __mulsi3
int __mulsi3(int a, int b) {
    int result = 0;
    for (int i = 0; i < b; i++) {
        result += a;
    }
    return result;
}

// Manual implementation of __divsi3
int __divsi3(int a, int b) {
    int count = 0;
    while (a >= b) {
        a -= b;
        count++;
    }
    return count;
}


// #include <math.h>  // For using pow and other mathematical functions

void MathCalculateAndDisplayResults() {
    ClearScreen();
    PutString("Ballistics Data:");
    SetCursor(2, 1, CURSOR_MODE_HIDE);
    if(g_G7Coefficient < 0){
        // Early exit if the computation fails
        PutString("Error: G7 coeff is error\n");
        return;  
    }
    // Ensure non-zero to prevent division by zero
    if(g_MuzzleVelocity <= 0){  
        PutString("Error: g_MuzzleVelocity is error\n");
        return;  
    }
    int range, drop;
    for (range = 25; range <= 500; range += 25) {
         // Direct multiplication
        int range_squared = range * range; 
         // Direct division
        drop = (range_squared * g_G7Coefficient) / g_MuzzleVelocity; 
        if (drop < 0) {
            PutString("Error: Drop calculation failed\n");
            return;  
        }
        char line[64];
        sprintf(line, "%4d m: Drop = %4d cm\n", range, drop);
        PutString(line);
    }
}


void CalculateAndDisplayResults() {
    ClearScreen();
    PutString("Ballistics Data:");
    SetCursor(2, 1, CURSOR_MODE_HIDE);
    if(g_G7Coefficient < 0){
        PutString("Error: G7 coeff is error\n");
        return;  
    }
    if(g_MuzzleVelocity < 0){
        PutString("Error: g_MuzzleVelocity is error\n");
        return;  
    }
    int range, drop;
    for (range = 25; range <= 500; range += 25) {
        int range_squared = multiply(range, range);
        if (range_squared < 0) {
            PutString("Error: Range squared is negative\n");
            return;  
        }
        drop = divide(multiply(range_squared, g_G7Coefficient), g_MuzzleVelocity);
        if (drop < 0) {
            PutString("Error: Drop calculation failed\n");
            return;  
        }
        char line[64];
        sprintf(line, "%4d m: Drop = %4d cm\n", range, drop);
        PutString(line);
    }
}

void ProcessMessage(Message_e message, uint32_t param, uint32_t* status) {
    *status = 0;
    //int input_g_G7Coefficient;
    switch(message) {
        case MSG_SETFOCUS:
            ClearScreen();
            PutStringCentered(1,"Enter G7 Coeff (e.g., 690 for 0.69):");
            SetCursor(2, 1, CURSOR_MODE_SHOW);
            g_CurrentInputField = 0;
            memset(g_InputBuffer, 0, sizeof(g_InputBuffer));
            break;

        case MSG_CHAR:
            if (DEBUG_CHARS) {
            // Buffer debug msgs
            char debugOutput[128]; 
            sprintf(debugOutput, "Received char: %c\n", (int)param);
            PutString(debugOutput);
            }
            if (param == '\n' || param == '\r') {
                if (g_CurrentInputField == 0) {
                    g_G7Coefficient = parse_fixed(g_InputBuffer);
                    //g_G7Coefficient = divide(input_g_G7Coefficient,1000);
                    g_CurrentInputField = 1;
                    ClearScreen();
                    PutStringCentered(1, "Enter Muzzle Velocity (m/s):");
                    SetCursor(2, 1, CURSOR_MODE_SHOW);
                    memset(g_InputBuffer, 0, sizeof(g_InputBuffer));
                } else if (g_CurrentInputField == 1) {
                    g_MuzzleVelocity = parse_fixed(g_InputBuffer);
                    CalculateAndDisplayResults();
                    g_CurrentInputField = 0;
                }
            } else if (param == '\b' && strlen(g_InputBuffer) > 0) {
                g_InputBuffer[strlen(g_InputBuffer) - 1] = '\0';
                PutChar('\b');
            } else if (is_alphanumeric(param) && strlen(g_InputBuffer) < sizeof(g_InputBuffer) - 1) {
                g_InputBuffer[strlen(g_InputBuffer)] = param;
                g_InputBuffer[strlen(g_InputBuffer) + 1] = '\0';
                PutChar(param);
            }
            break;

        case MSG_KEY:
            // TODO: handle arrow keys or special keys
            break;

        default:
            break;
    }
}