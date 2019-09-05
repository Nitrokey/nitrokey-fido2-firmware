
#ifndef USER_FEEDBACK_H
#define USER_FEEDBACK_H

#include "stdint.h"

// Delay in milliseconds to wait for user input
#define U2F_MS_USER_INPUT_WAIT				100


int8_t u2f_get_user_feedback();
int8_t u2f_get_user_feedback_extended_wipe();


#endif //CONOR_SOLO_USER_FEEDBACK_H
