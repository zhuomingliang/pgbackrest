/***********************************************************************************************************************************
Wait Handler
***********************************************************************************************************************************/
#include "common/memContext.h"
#include "common/time.h"
#include "common/wait.h"

/***********************************************************************************************************************************
Contains information about the wait handler
***********************************************************************************************************************************/
struct Wait
{
    MemContext *memContext;                                         // Context that contains the wait handler
    TimeUSec waitTime;                                              // Total time to wait (in usec)
    TimeUSec sleepTime;                                             // Next sleep time (in usec)
    TimeUSec sleepPrevTime;                                         // Previous time slept (in usec)
    TimeUSec beginTime;                                             // Time the wait began (in epoch usec)
};

/***********************************************************************************************************************************
New wait handler
***********************************************************************************************************************************/
Wait *
waitNew(double waitTime)
{
    // Make sure wait time is valid
    if (waitTime < 0.1 || waitTime > 999999.0)
        THROW(AssertError, "waitTime must be >= 0.1 and <= 999999.0");

    // Allocate wait object
    Wait *this = NULL;

    MEM_CONTEXT_NEW_BEGIN("wait")
    {
        // Create object
        this = memNew(sizeof(Wait));
        this->memContext = MEM_CONTEXT_NEW();

        // Store time
        this->waitTime = (TimeUSec)(waitTime * USEC_PER_SEC);

        // Calculate first sleep time -- start with 1/10th of a second for anything >= 1 second
        if (this->waitTime >= USEC_PER_SEC)
            this->sleepTime = USEC_PER_SEC / 10;
        // Unless the wait time is really small -- in that case divide wait time by 10
        else
            this->sleepTime = this->waitTime / 10;

        // Get beginning time
        this->beginTime = timeUSec();
    }
    MEM_CONTEXT_NEW_END();

    return this;
}

/***********************************************************************************************************************************
Wait and return whether the caller has more time left
***********************************************************************************************************************************/
bool
waitMore(Wait *this)
{
    bool result = false;

    // If sleep is 0 then the wait time has already ended
    if (this->sleepTime > 0)
    {
        // Sleep required amount
        sleepUSec(this->sleepTime);

        // Get the end time
        TimeUSec elapsedTime = timeUSec() - this->beginTime;

        // Is there more time to go?
        if (elapsedTime < this->waitTime)
        {
            // Calculate sleep time as a sum of current and last (a Fibonacci-type sequence)
            TimeUSec sleepNextTime = this->sleepTime + this->sleepPrevTime;

            // Make sure sleep time does not go beyond end time (this won't be negative because of the if condition above)
            if (sleepNextTime > this->waitTime - elapsedTime)
                sleepNextTime = this->waitTime - elapsedTime;

            // Store new sleep times
            this->sleepPrevTime = this->sleepTime;
            this->sleepTime = sleepNextTime;
        }
        // Else set sleep to zero so next call will return false
        else
            this->sleepTime = 0;

        // Need to wait more
        result = true;
    }

    return result;
}

/***********************************************************************************************************************************
Free the wait
***********************************************************************************************************************************/
void
waitFree(Wait *this)
{
    memContextFree(this->memContext);
}
