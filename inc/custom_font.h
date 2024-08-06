#ifndef _CUSTOM_FONT_H_
#define _CUSTOM_FONT_H_

// 6 x 12 px font based on Roboto Mono

/**
 * @brief Displays a text centered in screen on the selected line.
 *
 * @param lineNo Line number where to display, from 1 to 5, from up to down.
 * @param text Text to display, maximum 21 characters. Null terminated
 */
int DisplayRobotoTextCenter(unsigned short lineNo, const char *text);

/**
 * @brief Displays a text aligned to the left on the selected line.
 *
 * @param lineNo Line number where to display, from 1 to 5, from up to down.
 * @param text Text to display, maximum 21 characters. Null terminated
 */
int DisplayRobotoTextLeft(unsigned short lineNo, const char *text);

/**
 * @brief Displays a text aligned to the right on the selected line.
 *
 * @param lineNo Line number where to display, from 1 to 5, from up to down.
 * @param text Text to display, maximum 21 characters. Null terminated
 */
int DisplayRobotoTextRight(unsigned short lineNo, const char *text);

#endif
