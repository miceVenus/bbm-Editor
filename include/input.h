#ifndef INPUT_H
#define INPUT_H

#define CTRL_KEY(k) ((k) & 0x1f)

#endif
void ProcessKeypress();
void WaringExitWithoutSave(int quitTime);
void MoveCursor(int key);
void HandleEnter();
int ReadKeypress();