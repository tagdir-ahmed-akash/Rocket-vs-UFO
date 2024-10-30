#include <Windows.h>
#include <mmsystem.h>  // Include for PlaySound
#include <graphics.h>
#include <conio.h>
#include <time.h>
#include "EasyXPng.h" 
#include <chrono>  
#include <vector>
#include <cstdlib> // For random numbers
#include <ctime> // For seeding rand

#pragma comment(lib, "winmm.lib")
#pragma warning(default: 4996)




#define WIDTH  640     // Updated dimensions for rotated image
#define HEIGHT 480

int dy = 0;
int speed = 3; // Speed for background scrolling

// Function to rotate an image 90 degrees clockwise
void rotateImage90(IMAGE* src, IMAGE* dst) {
    DWORD* srcData = GetImageBuffer(src);
    DWORD* dstData = GetImageBuffer(dst);
    int srcWidth = src->getwidth();
    int srcHeight = src->getheight();

    for (int y = 0; y < srcHeight; y++) {
        for (int x = 0; x < srcWidth; x++) {
            // Rotate 90 degrees clockwise
            dstData[x * srcHeight + (srcHeight - 1 - y)] = srcData[y * srcWidth + x];
        }
    }
}

void showRotatedBackground(IMAGE& im_rotated) {
    // Draw the scrolling rotated background without visible seam
    putimage(0, dy - WIDTH, &im_rotated);   // Draw top part
    putimage(0, dy, &im_rotated);           // Draw bottom part

    // Update scrolling position
    dy += speed;
    if (dy >= WIDTH) dy = 0;  // Reset after a full scroll
}

class Bullet {
public:
    IMAGE im_bullet;
    float x, y; // Position of the bullet
    float speedX, speedY; // Speed in x and y directions
    bool isCrossed; // Whether the bullet has crossed the bottom
    DWORD crossTime; // Time when the bullet crossed

    Bullet(float startX) : x(startX), y(0), isCrossed(false), crossTime(0) {
        loadimage(&im_bullet, _T("bullet.png")); // Load bullet image
        speedX = (rand() % 3 + 2) * (rand() % 2 == 0 ? 1 : -1); // Random horizontal speed (2 to 4)
        speedY = (rand() % 3 + 2); // Random downward speed (2 to 4)
    }

    void update() {
        // Update bullet position
        x += speedX; // Update horizontal position
        y += speedY; // Update vertical position

        // Check for horizontal edge collisions
        if (x <= 0) {
            x = 0; // Prevent going out of left boundary
            speedX = -speedX; // Reverse horizontal direction
        }
        else if (x >= WIDTH - im_bullet.getwidth()) {
            x = WIDTH - im_bullet.getwidth(); // Prevent going out of right boundary
            speedX = -speedX; // Reverse horizontal direction
        }

        // Check if the bullet has crossed the bottom edge
        if (y >= HEIGHT) {
            if (!isCrossed) {
                isCrossed = true; // Mark that the bullet has crossed
                crossTime = GetTickCount(); // Record the time it crossed
            }
        }

        // If the bullet has crossed the bottom edge, check if 5 seconds have passed
        if (isCrossed && (GetTickCount() - crossTime >= 5000)) {
            // Reset the bullet to the top with a random horizontal position
            x = rand() % (WIDTH - 10) + 5; // Random horizontal start position
            y = 0; // Reset vertical position
            isCrossed = false; // Reset crossing flag
        }
    }

    void draw() {
        putimagePng(x - im_bullet.getwidth() / 2, y, &im_bullet); // Draw the bullet
    }
};

class Rocket {
public:
    IMAGE im_rocket;
    float x, y; // Position of the rocket
    float width, height; // Dimensions of the rocket

    Rocket() : x(WIDTH / 2), y(HEIGHT / 2) { // Initialize position to center
        loadimage(&im_rocket, _T("rocket.png")); // Load rocket image
        width = im_rocket.getwidth();
        height = im_rocket.getheight();
    }

    void draw() {
        putimagePng(x - width / 2, y - height / 2, &im_rocket); // Draw the rocket
    }

    void update(float mx, float my) {
        x = mx; // Update x position
        y = my; // Update y position
    }

    bool checkCollision(Bullet& bullet) {
        // Simple bounding box collision detection
        return x - width / 2 < bullet.x + bullet.im_bullet.getwidth() &&
            x + width / 2 > bullet.x &&
            y - height / 2 < bullet.y + bullet.im_bullet.getheight() &&
            y + height / 2 > bullet.y;
    }
};

class UFO {
public:
    IMAGE im_ufo;
    float x, y; // Position of the UFO
    float speed; // Speed of the UFO

    UFO() : speed(2.0f) { // Initialize speed
        loadimage(&im_ufo, _T("ufo.png")); // Load UFO image
        x = rand() % (WIDTH - im_ufo.getwidth()); // Random horizontal start position
        y = 0; // Start from the top
    }

    void update(float rocketX, float rocketY) {
        // Move the UFO towards the rocket's position
        if (x < rocketX) x += speed;
        else if (x > rocketX) x -= speed;

        // Move down towards the rocket's vertical position
        if (y < rocketY) y += speed;
        else if (y > rocketY) y -= speed;
    }

    void draw() {
        putimagePng(x - im_ufo.getwidth() / 2, y - im_ufo.getheight() / 2, &im_ufo); // Draw the UFO
    }

    bool checkCollision(Rocket& rocket) {
        // Simple bounding box collision detection
        return x - im_ufo.getwidth() / 2 < rocket.x + rocket.width / 2 &&
            x + im_ufo.getwidth() / 2 > rocket.x - rocket.width / 2 &&
            y - im_ufo.getheight() / 2 < rocket.y + rocket.height / 2 &&
            y + im_ufo.getheight() / 2 > rocket.y - rocket.height / 2;
    }
};

class Heart {
public:
    IMAGE im_heart;
    float x, y; // Position of the heart

    Heart(float startX, float startY) : x(startX), y(startY) {
        loadimage(&im_heart, _T("heart.png")); // Load heart image
    }

    void draw() {
        putimagePng(x, y, &im_heart); // Draw the heart
    }
};

void playExplosionSound() {
    PlaySound(TEXT("explode.mp3"), NULL, SND_ASYNC | SND_FILENAME);
}

// Function to play music
void playMusic() {
    mciSendString(TEXT("open \"game_music.mp3\" type mpegvideo alias bgm"), NULL, 0, NULL);
    mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
}

// Function to stop music
void stopMusic() {
    mciSendString(TEXT("stop bgm"), NULL, 0, NULL);
    mciSendString(TEXT("close bgm"), NULL, 0, NULL);
}
void displayGameOver() {

    mciSendString(TEXT("open explode.mp3 alias explosion"), NULL, 0, NULL);
    mciSendString(TEXT("play explosion"), NULL, 0, NULL);
    settextcolor(RED); // Set text color
    settextstyle(48, 0, _T("Arial")); // Set font size and style
    outtextxy(WIDTH / 2 - 100, HEIGHT / 2, _T("Game Over!")); // Display "Game Over!" message
}

int main()
{
    // Seed random number generator
    srand(static_cast<unsigned int>(time(0)));

    IMAGE im_bk, im_rotated;
    loadimage(&im_bk, _T("bg.png"));          // Load original background image

    // Initialize graphics window to match rotated dimensions
    initgraph(HEIGHT, WIDTH);

    // Create an image to store the rotated background
    im_rotated = IMAGE(HEIGHT, WIDTH);
    rotateImage90(&im_bk, &im_rotated);       // Rotate the original background

    Rocket rocket; // Create a Rocket instance
    UFO ufo; // Create a UFO instance
    std::vector<Bullet> bullets; // Vector to store bullets
    std::vector<Heart> hearts; // Vector to store hearts
    int lives = 5; // Number of hearts (lives)

    // Initialize heart positions
    for (int i = 0; i < lives; ++i) {
        hearts.emplace_back(10 + i * 30, 10); // Position hearts in the top left
    }

    // Load blowup image
    IMAGE im_blowup;
    loadimage(&im_blowup, _T("blowup.png"));

    // Timer variables
    DWORD lastDropTime = GetTickCount();
    DWORD blowupTime = 0; // Time when blowup is displayed
    bool isBlowupVisible = false; // Flag to check blowup visibility

    BeginBatchDraw();
    while (lives > 0) { // Run the game loop while lives are more than 0
        showRotatedBackground(im_rotated);    // Display the rotated background

        // Get mouse position
        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(GetHWnd(), &mousePos); // Convert to client coordinates
        rocket.update(mousePos.x, mousePos.y); // Update rocket position

        // Update and draw UFO
        ufo.update(rocket.x, rocket.y);
        ufo.draw();

        // Check for collision between UFO and Rocket
        if (ufo.checkCollision(rocket)) {
            lives = 0; // Set lives to 0 to trigger game over
            playExplosionSound(); // Play explosion sound
            blowupTime = GetTickCount(); // Record the blowup time
            isBlowupVisible = true; // Set flag to show blowup
        }

        // Update and draw each bullet
        for (size_t i = 0; i < bullets.size(); ++i) {
            bullets[i].update(); // Update bullet position
            bullets[i].draw();   // Draw bullet

            // Check for collision with the rocket
            if (rocket.checkCollision(bullets[i])) {
                bullets.erase(bullets.begin() + i); // Remove the bullet
                if (lives > 0) {
                    lives--; // Decrease life
                    playExplosionSound(); // Play explosion sound
                    blowupTime = GetTickCount(); // Record the blowup time
                    isBlowupVisible = true; // Set flag to show blowup
                }
                --i; // Adjust index after removal
            }
        }

        // Draw hearts
        for (int i = 0; i < lives; ++i) {
            hearts[i].draw(); // Draw each heart
        }

        // Draw the rocket on top of the background
        rocket.draw();

        // Display the blowup effect if it should be visible
        if (isBlowupVisible && (GetTickCount() - blowupTime < 500)) {
            putimagePng(rocket.x - 20, rocket.y - 20, &im_blowup);
        }
        else if (isBlowupVisible) {
            isBlowupVisible = false; // Reset the visibility flag after the time has passed
        }

        // Check if it's time to drop a new bullet
        DWORD currentTime = GetTickCount();
        if (currentTime - lastDropTime >= 2000) { // Drop bullet every 2 seconds
            float startX = rand() % (WIDTH - 10) + 5; // Random start X position (5 to WIDTH-5)
            bullets.emplace_back(startX); // Create a new bullet at a random horizontal position
            lastDropTime = currentTime; // Reset the timer
        }

        if (lives <= 0) {
            displayGameOver(); // Show "Game Over!" message
        }

        Sleep(10); // Control frame rate
        playMusic();
        FlushBatchDraw(); // Refresh the screen
    }
    EndBatchDraw();

    _getch();
    closegraph(); // Close the graphics window
    return 0;
}
