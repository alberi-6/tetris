#include <iostream>
#include <ncurses.h>
#include <thread>
#include <vector>

using namespace std;

int nScreenWidth = 25;			// Console Screen Size X (columns)
int nScreenHeight = 18;			// Console Screen Size Y (rows)
string tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr; // for storing the elements of the field


int Rotate(int px, int py, int r) {
    switch (r % 4) {
        case 0: return py * 4 + px;         // 0 degrees
        case 1: return 12 + py - (px * 4);  // 90 degrees
        case 2: return 15 - (py * 4) - px;  // 180 degrees
        case 3: return 3 - py + (px * 4);   // 270 degrees
    }
    return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
    // PosX, PosY top left corner coordinates of 4x4 tetromino in field
    for (int px = 0; px < 4; px ++) {
        for (int py = 0; py < 4; py ++) {
            // Get index into piece
            int pi = Rotate(px, py, nRotation);
            
            // Get index into field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);
            
            if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
                    if (tetromino[nTetromino][pi] == 'X' && pField[fi] != 0)
                        return false; // fail on first hit
                }
            }
        }
    }

    return true;
}


int main() {
    // Create assets
    tetromino[0].append("..X.");
    tetromino[0].append("..X.");
    tetromino[0].append("..X.");
    tetromino[0].append("..X.");
    
    tetromino[1].append("..X.");
    tetromino[1].append(".XX.");
    tetromino[1].append(".X..");
    tetromino[1].append("....");
    
    tetromino[2].append(".X..");
    tetromino[2].append(".XX.");
    tetromino[2].append("..X.");
    tetromino[2].append("....");
    
    tetromino[3].append("....");
    tetromino[3].append(".XX.");
    tetromino[3].append(".XX.");
    tetromino[3].append("....");
    
    tetromino[4].append("..X.");
    tetromino[4].append(".XX.");
    tetromino[4].append("..X.");
    tetromino[4].append("....");
    
    tetromino[5].append("....");
    tetromino[5].append(".XX.");
    tetromino[5].append("..X.");
    tetromino[5].append("..X.");
    
    tetromino[6].append("....");
    tetromino[6].append(".XX.");
    tetromino[6].append(".X..");
    tetromino[6].append(".X..");
    
    pField = new unsigned char[nFieldWidth * nFieldHeight]; // Create play-field
    for (int x = 0; x < nFieldWidth; x++) // Board boundary
        for (int y = 0; y < nFieldHeight; y++)
            pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0; // 9 represents border
    
    // char *screen = new char[nScreenWidth*nScreenHeight];
    // for (int i = 0; i < nScreenWidth*nScreenHeight; i++) screen[i] = ' ';
    
    initscr();
    cbreak(); // Pressing control + C lets you exit out of your ncurses program
    noecho(); // whatever window input does not show up on the screen
    
    WINDOW *my_win = newwin(nScreenHeight, nScreenWidth, 0, 0);
    refresh(); // refreshes whole screen
    nodelay(my_win, true);
    keypad(my_win, true);
    curs_set(0);
    
    wrefresh(my_win); // refreshes specific window
    
    // Game Logic Stuff
    bool bGameOver = false;
    char buffer;
    
    int nCurrentPiece = rand() % 7;;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2 - 2;
    int nCurrentY = 0;
    
    bool bKey[4];
    int pressedKey;
    bool bRotateHold = false;
    int allowedKeys[4] = {KEY_RIGHT, KEY_LEFT, KEY_DOWN, 122};
    // 122 -> ascii of z key

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;
    int nScore = 0;

    vector<int> vLines;
    
    while (!bGameOver) {
        // GAME TIMING =========
        this_thread::sleep_for(50ms); // Game Tick
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);
        
        // INPUT ===============
        pressedKey = wgetch(my_win);
        for (int k = 0; k < 4; k++) {
            bKey[k] = pressedKey == allowedKeys[k];
        }
        
        // GAME LOGIC ==========
        nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;

        nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;

        nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
        
        if (bKey[3]) {
            nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        else
            bRotateHold = false;
        
        if (bForceDown) {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
                nCurrentY++;
            else {
                // Lock the current piece into the field
                for (int px = 0; px < 4; px ++) {
                    for (int py = 0; py < 4; py ++) {
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == 'X') {
                            buffer = nCurrentPiece + 1;
                            // Make tetromino part of field
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = buffer;
                        }
                    }
                }

                nPieceCount++;
                if (nPieceCount % 10 == 0)
                    if (nSpeed >= 10) nSpeed--;

                // Check if we have got any lines
                for (int py = 0; py < 4; py++)
                    if (nCurrentY + py < nFieldHeight - 1) {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth -1; px++)
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
                            // a &= b --> a = a & b (bitwise and)

                        if (bLine) {
                            // Remove Line, set to = (no. 8)
                            for (int px = 1; px < nFieldWidth -1; px++)
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;

                            vLines.push_back(nCurrentY + py); // add row which now contains lines
                        }
                    }

                nScore += 25;
                if (!vLines.empty()) nScore += (1 << vLines.size()) * 100; // exponential increase if multiple lines at the same time

                // choose next piece
                nCurrentPiece = rand() % 7;
                nCurrentRotation = 0;
                nCurrentX = nFieldWidth / 2 - 2;
                nCurrentY = 0;

                // if next piece does not fit
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

            }
            nSpeedCounter = 0;
        }

        // clear out any other characters that have been buffered
        flushinp();

        // RENDER OUTPUT =======
        
    
        // Draw field and display frame
        for (int x = 0; x < nFieldWidth; x++)
            for (int y = 0; y < nFieldHeight; y++) {
                buffer = " ABCDEFG=#"[pField[y*nFieldWidth + x]];
                mvwprintw(my_win, y, x, "%c", buffer);
            }

        // Draw current Piece on top of field
        for (int px = 0; px < 4; px ++) {
            for (int py = 0; py < 4; py ++) {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == 'X') {
                    buffer = nCurrentPiece + int('A'); 
                    // ascii value of A (65). Then B -> 66, C -> 67 etc.
                    mvwprintw(my_win, (nCurrentY + py), (nCurrentX + px), "%c", buffer);
                }
            }
        }

    // Draw Score
    mvwprintw(my_win, 0, nFieldWidth + 2, "SCORE: %d", nScore);

    if (!vLines.empty()) {
        // Display Frame (cheekily to draw lines)
        wrefresh(my_win);
        this_thread::sleep_for(400ms); // Delay a bit

        // remove lines
        for (auto &v : vLines)
            for (int px = 1; px < nFieldWidth - 1; px++) {
                for (int py = v; py > 0; py--)
                    pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px]; // move field down one step for each "=" line
                pField[px] = 0; // new top row
            }

        vLines.clear();
        
    }

        // Display frame
        wrefresh(my_win);
    }
    
    endwin();
    cout << "Game Over!! Score: " << nScore << endl;
    
    return 0;
}