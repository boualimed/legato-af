#include <stdio.h>
#include <poll.h>
#include "legato.h"
#include "le_sms_interface.h"

static void SmsRxHandler
(
    le_sms_MsgRef_t msg,
    void* contextPtr // not used
)
{
    char textBuffer[1024];

    le_sms_GetSenderTel(msg, textBuffer, sizeof(textBuffer));

    switch (le_sms_GetFormat(msg))
    {
        case LE_SMS_FORMAT_PDU:
            printf("Received SMS in PDU format from %s.\n", textBuffer);
            break;

        case LE_SMS_FORMAT_TEXT:
            printf("Received text SMS from %s: ", textBuffer);
            le_sms_GetText(msg, textBuffer, sizeof(textBuffer));
            printf("%s\n", textBuffer);
            break;

        case LE_SMS_FORMAT_BINARY:
            printf("Received SMS in binary format from %s.\n", textBuffer);
            break;

        case LE_SMS_FORMAT_UNKNOWN:
            printf("Received SMS in unknown format from %s.\n", textBuffer);
            break;
    }

    le_sms_Delete(msg);
}

int main(int argc, char** argv)
{
    struct pollfd pollControl;

    le_sms_ConnectService();

    // Register a callback function to be called when an SMS arrives.
    (void)le_sms_AddRxMessageHandler(SmsRxHandler, NULL);

    printf("Waiting for SMS messages to arrive...\n");

    // Get the Legato event loop "readyness" file descriptor and put it in a pollfd struct
    // configured to detect "ready to read".
    pollControl.fd = le_event_GetFd();
    pollControl.events = POLLIN;

    while (true)
    {
        // Block until the file descriptor is "ready to read".
        int result = poll(&pollControl, 1, -1);

        if (result > 0)
        {
            // The Legato event loop needs servicing. Keep servicing it until there's nothing left.
            while (le_event_ServiceLoop() == LE_OK)
            {
                /* le_event_ServiceLoop() has more to do.  Need to call it again. */
            }
        }
        else
        {
            LE_FATAL("poll() failed with errno %m.");
        }
    }
}

