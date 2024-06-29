#pragma once


DWORD getCurrentCr3();

void saveScreen();

void restoreScreen();

int setVideoMode(int mode);

int setGraphMode(int mode);