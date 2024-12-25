#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <fstream> // For file operations

using namespace std;

static bool allowMove = false;
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food
{
public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game
{
public:
    Snake snake;
    Food food;
    bool running;
    int score;
    int highScore; // Variable to store the high score

    Game() : snake(), food(snake.body), running(true), score(0), highScore(0)
    {
        InitAudioDevice();
        LoadHighScore(); // Load high score from file
    }

    ~Game()
    {
        SaveHighScore(); // Save high score to file
        CloseAudioDevice();
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
        if (!running) DrawGameOver();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisions();
        }
    }

    void DrawGameOver()
    {
        DrawText("Game Over!", offset + cellSize * cellCount / 2 - 100, offset + cellSize * cellCount / 2 - 40, 40, darkGreen);
        DrawText(TextFormat("Final Score: %i", score), offset + cellSize * cellCount / 2 - 100, offset + cellSize * cellCount / 2 + 10, 40, darkGreen);
        DrawText(TextFormat("High Score: %i", highScore), offset + cellSize * cellCount / 2 - 100, offset + cellSize * cellCount / 2 + 60, 20, darkGreen);
        DrawText("Press SPACE to Restart", offset + cellSize * cellCount / 2 - 150, offset + cellSize * cellCount / 2 + 100, 20, darkGreen);
    }

    void CheckCollisions()
    {
        CheckCollisionWithFood();
        CheckCollisionWithEdges();
        CheckCollisionWithTail();
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            // Update high score if current score exceeds it
            if (score > highScore)
            {
                highScore = score;
            }
            // Play sound for eating food if needed
        }
    }

    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x >= cellCount || snake.body[0].x < 0 || 
            snake.body[0].y >= cellCount || snake.body[0].y < 0)
        {
            GameOver();
        }
    }

    void GameOver()
    {
        running = false;
        // Play sound for game over if needed
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    void Restart()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = true;
        score = 0;
    }

    void LoadHighScore()
    {
        ifstream file("highscore.txt");
        if (file.is_open())
        {
            file >> highScore;
            file.close();
        }
    }

    void SaveHighScore()
    {
        ofstream file("highscore.txt");
        if (file.is_open())
        {
            file << highScore;
            file.close();
        }
    }
};

int main()
{
    cout << "Starting the game..." << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose())
    {
        BeginDrawing();

        if (EventTriggered(0.2))
        {
            allowMove = true;
            game.Update();
        }

        if (game.running)
        {
            if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && allowMove)
            {
                game.snake.direction = {0, -1};
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove)
            {
                game.snake.direction = {0, 1};
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && allowMove)
            {
                game.snake.direction = {-1, 0};
                allowMove = false;
            }
            if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove)
            {
                game.snake.direction = {1, 0};
                allowMove = false;
            }
        }
        else
        {
            if (IsKeyPressed(KEY_SPACE)) // Restart the game
            {
                game.Restart();
            }
        }

        // Drawing
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
        DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("Score: %i", game.score), offset + cellSize * cellCount - 150, 20, 40, darkGreen);
        game.Draw();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
