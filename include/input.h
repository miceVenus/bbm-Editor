#ifndef INPUT_H
#define INPUT_H
#endif

#define CTRL_KEY(k) ((k) & 0x1f)

void ProcessKeypress();
void WaringExitWithoutSave(int quitTime);
void MoveCursor(int key);
void HandleEnter();