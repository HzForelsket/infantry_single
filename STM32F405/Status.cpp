#include "Status.h"
#include "can.h"
bool Status::can_test(CAN* here_can)
{
    if(this->m_CAN)
        return (here_can->ready);
}

void Status::init(bool canTestOpen, bool uartTestOpen)
{
    this->m_CAN = canTestOpen;
    //this->uartTest = uartTestOpen;
    this->sumTest = canTestOpen + uartTestOpen;
}