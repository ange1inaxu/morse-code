# Overview
[demo video](https://youtu.be/OhuV91XRvXM)


I created two state machines: one for button pushes and another for the posting to the web app. I also created four variables: `morse_code` (representing the sequence of . and _ displayed on the TFT screen), `current_letter` (representing the letter or digit the user is currently encoding), `deciphered` (representing the translated English message from the Morse Code), and `valid` (1 for a valid Morse Code translation and 0 for an invalid Morse Code translation). I initiated `valid=1`.

For the button FSM, button_state=0 represents being idle. In this state, the following would occur if the specified buttons were pushed:
    BUTTON1: `.` is concatenated to `morse_code` and `current_letter`, and progress button_state=1
    BUTTON2: `_` is concatenated to `morse_code` and `current_letter`, and progress button_state=2
    BUTTON3: Check if the last BUTTON3 press `last_button3_read` was less than 1000 ms ago (ie. a double click); if it was, ` ` is concatenated to `morse_code` and `deciphered`, and we set `last_button3_read=millis()` to keep track of another double click for a space, and progress button_state=3. 

In each of button_state=1 and button_state=2, we check if BUTTON1 or BUTTON2 is released. If they are, then we return to the original button_state=0 and wait for the next button push. In button_state=3, we check if BUTTON3 is released and call on a `map_letter()` function. In this helper function, we check if `current_letter` maps to any of the Morse Code symbols, as given in the design exercise instructions. If they are, we concatenate the character to `deciphered`. However, in the case that the sequence is invalid, we set `valid=0`. In addition, we memset `current_letter` as the user begins typing the next character.

After the button FSM, we print the Morse Code to the TFT screen. Next, we enter the post FSM, where `post_state` is initialized as 0, the idle state. If BUTTON4 is pushed, post_state progresses to 1. If BUTTON4 is released, post_state is progressed to 2. Provided that `valid=1` (ie. all letters were valid Morse Code translations) and the length of `deciphered` is greater than 0 (because we don't want to spam the web page with empty messages), then we do the HTTP POST request. After the message is posted, "POSTED" is also printed to the LCD screen so the user knows that the message went through. If these posting criteria are not satisfied (ie. `valid=0` and/or `strlen(deciphered)=0`), then "INVALID" is printed to the screen so that the user is not confused when they do not see their message posted. Afterwards, we memset `morse_code`, `current_letter`, and `deciphered`, reset valid=1, and post_state=0.
