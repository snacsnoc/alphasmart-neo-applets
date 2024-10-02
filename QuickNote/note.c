#include "os3k.h"

APPLET_HEADER_BEGIN
    APPLET_ID(0xB3B2)
    APPLET_NAME("Quick Note")
    APPLET_INFO("Type and store a quick note")
    APPLET_VERSION(1, 0, "A")
    APPLET_LANGUAGE_EN_US
APPLET_HEADER_END

#define MAX_NOTE_LENGTH 128
static char noteBuffer[MAX_NOTE_LENGTH];
static int noteLength = 0;

void ProcessMessage(Message_e message, uint32_t param, uint32_t* status) {
    *status = 0;
    switch (message) {
        case MSG_INIT:
            ClearScreen();
            SetCursor(0, 0, CURSOR_MODE_SHOW);
            PutString("Quick Note Applet: Type your note.");
            noteLength = 0;
            memset(noteBuffer, 0, sizeof(noteBuffer));
            break;

        case MSG_SETFOCUS:
            ClearScreen();
            PutStringCentered(1, "Type your note and press ENTER to save.");
            SetCursor(2, 0, CURSOR_MODE_SHOW);
            break;

        case MSG_CHAR:
            if (noteLength < MAX_NOTE_LENGTH - 1) {
                char ch = (char)param;
                if (ch == KEY_ENTER) {
                    noteBuffer[noteLength] = '\0'; 
                    ClearScreen();
                    PutStringCentered(1, "Note saved! Press any key to see it.");
                } else {
                    noteBuffer[noteLength++] = ch;
                    PutString(ch);  
                }
            }
            break;

        case MSG_KEY:
            if (param == KEY_ENTER) {
                DialogInit(true, 3, 5, 10);
                DialogAddItem(noteBuffer, strlen(noteBuffer), ' ', 0, KEY_NONE, 0);
                DialogRun();
            }
            break;

        default:
            break;
    }
}
