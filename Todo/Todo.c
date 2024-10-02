#include "os3k.h"

APPLET_HEADER_BEGIN
    APPLET_ID(0xA2A1)
    APPLET_NAME("Daily Planner")
    APPLET_INFO("To Do List Applet")
    APPLET_VERSION(1, 1, 0)
    APPLET_LANGUAGE_EN_US
APPLET_HEADER_END

#define MAX_TASKS 50
#define TASK_LENGTH 40
#define MAX_DISPLAY_TASKS 10

typedef struct {
    char text[TASK_LENGTH];
    bool completed;
} Task;

Task tasks[MAX_TASKS];
int task_count = 0;
// For scrolling
int display_start = 0;  

void DisplayTasks(void);
void HandleKey(Key_e key);
//char TranslateKeyToChar(KeyMod_e key);

void ProcessMessage(Message_e message, uint32_t param, uint32_t* status) {
    *status = 0;
    switch(message) {
        case MSG_INIT:
            ClearScreen();
            SetCursorMode(CURSOR_MODE_SHOW);
            break;
        case MSG_SETFOCUS:
            DisplayTasks();
            break;
        case MSG_KEY:
            HandleKey((Key_e)param);
            break;
    }
}

void DisplayTasks() {
    ClearScreen();
    for (int i = display_start; i < task_count && i < display_start + MAX_DISPLAY_TASKS; i++) {
        PutString(tasks[i].completed ? "[x] " : "[ ] ");
        PutString(tasks[i].text);
        PutChar('\n');
    }
}

void HandleKey(Key_e key) {
    static char buffer[TASK_LENGTH];
    static int buffer_length = 0;
    char c; 

    switch(key) {
        case KEY_ENTER:
            if (buffer_length > 0) {
                strncpy(tasks[task_count].text, buffer, TASK_LENGTH);
                tasks[task_count].completed = false;
                task_count++;
                memset(buffer, 0, TASK_LENGTH);
                buffer_length = 0;
                DisplayTasks();
            }
            break;
        case KEY_BACKSPACE:
            if (buffer_length > 0) {
                buffer[--buffer_length] = '\0';
                DisplayTasks();
            }
            break;
        case KEY_UP:
            if (display_start > 0) {
                display_start--;
                DisplayTasks();
            }
            break;
        case KEY_DOWN:
            if (display_start < task_count - MAX_DISPLAY_TASKS) {
                display_start++;
                DisplayTasks();
            }
            break;
        case KEY_DELETE:
            if (task_count > 0) {  
                task_count--;
                DisplayTasks();
            }
            break;
        default:
            c = TranslateKeyToChar(key);  
            if (buffer_length < TASK_LENGTH - 1) {
                buffer[buffer_length++] = c;
                buffer[buffer_length] = '\0';
                DisplayTasks();
            }
            break;
    }
}


// char TranslateKeyToChar(KeyMod_e key) {
//     // Placeholder function to map KeyMod_e to char
//     return (char)key; 
// }
