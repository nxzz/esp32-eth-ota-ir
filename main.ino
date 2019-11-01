#include <IRremote.h>

IRrecv irrecv(35);

void OTASetup()
{
    Serial.println("OTASetup");
    irrecv.enableIRIn();
    pinMode(2, OUTPUT);
}

void OTALoop()
{
    decode_results results; // Somewhere to store the results
    if (irrecv.decode(&results))
    {                       // Grab an IR code
        dumpCode(&results);  // Blank line between entries
        Serial.println(""); // Blank line between entries
        irrecv.resume();    // Prepare for the next value
    }
}

void dumpCode(decode_results *results)
{
    // Dump data
    for (int i = 1; i < results->rawlen; i++)
    {
        Serial.print(results->rawbuf[i], DEC);
        // Serial.print(results->rawbuf[i] * USECPERTICK, DEC);
        if (i < results->rawlen - 1)
            Serial.print(","); // ',' not needed on last one
    }
}

void OTAStop()
{
    irrecv.disableIRIn();
}
