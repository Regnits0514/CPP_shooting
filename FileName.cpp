#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

class Bullet {
public:
    sf::RectangleShape shape;
    float speed = 10.f;

    Bullet(float x, float y) {
        shape.setSize({ 5.f, 15.f });
        shape.setFillColor(sf::Color::Yellow);
        shape.setPosition(x, y);
    }

    void update() {
        shape.move(0.f, -speed);
    }
};

class Enemy {
public:
    sf::RectangleShape shape;
    float speed;
    int hp;
    int type;

    Enemy(float x, float y, int type) : type(type) {
        shape.setSize({ 40.f, 40.f });
        shape.setPosition(x, y);

        switch (type) {
        case 0:
            shape.setFillColor(sf::Color::Red);
            speed = 2.f;
            hp = 1;
            break;
        case 1:
            shape.setFillColor(sf::Color(255, 165, 0)); // 주황색
            speed = 1.5f;
            hp = 2;
            break;
        case 2:
            shape.setFillColor(sf::Color::Yellow);
            speed = 1.0f;
            hp = 3;
            break;
        }
    }

    void update() {
        shape.move(0.f, speed);
    }

    bool isDead() const {
        return hp <= 0;
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Top-Down Shooter");
    window.setFramerateLimit(60);

    // Player setup
    sf::RectangleShape player;
    player.setSize({ 50.f, 50.f });
    player.setFillColor(sf::Color::Blue);
    player.setPosition(WINDOW_WIDTH / 2 - 25.f, WINDOW_HEIGHT - 60.f);

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;

    sf::Clock bulletClock;
    sf::Clock enemySpawnClock;

    // Score
    int score = 0;

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        return -1; // 실패 시 종료
    }

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10.f, 10.f);
    scoreText.setString("Score: 0");

    // Lives & Game Over setup (루프 밖에서 초기화)
    int lives = 3;
    bool isGameOver = false;

    sf::Text lifeText;
    lifeText.setFont(font);
    lifeText.setCharacterSize(24);
    lifeText.setFillColor(sf::Color::Green);
    lifeText.setPosition(10.f, 40.f);
    lifeText.setString("Lives: " + std::to_string(lives));

    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setStyle(sf::Text::Bold);
    gameOverText.setPosition(WINDOW_WIDTH / 2.f - 150.f, WINDOW_HEIGHT / 2.f - 50.f);
    gameOverText.setString("GAME OVER");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();

        if (!isGameOver) {
            // Player movement
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && player.getPosition().x > 0)
                player.move(-6.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && player.getPosition().x + player.getSize().x < WINDOW_WIDTH)
                player.move(6.f, 0.f);

            // Shooting
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && bulletClock.getElapsedTime().asMilliseconds() > 300) {
                bullets.emplace_back(player.getPosition().x + player.getSize().x / 2 - 2.5f, player.getPosition().y);
                bulletClock.restart();
            }

            // Enemy spawn
            if (enemySpawnClock.getElapsedTime().asSeconds() > 1.f) {
                float enemyX = static_cast<float>(rand() % (WINDOW_WIDTH - 40));
                int enemyType = rand() % 3; // 0~2 타입 중 무작위 선택
                enemies.emplace_back(enemyX, -40.f, enemyType);
                enemySpawnClock.restart();
            }

            // Update bullets
            for (auto& bullet : bullets)
                bullet.update();

            // Update enemies
            for (auto& enemy : enemies)
                enemy.update();

            // Remove off-screen bullets
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                [](Bullet& b) { return b.shape.getPosition().y + b.shape.getSize().y < 0; }),
                bullets.end());

            // Remove off-screen enemies
            enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                [](Enemy& e) { return e.shape.getPosition().y > WINDOW_HEIGHT; }),
                enemies.end());

            // Collision detection (bullet vs enemy)
            for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
                bool bulletRemoved = false;
                for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
                    if (bulletIt->shape.getGlobalBounds().intersects(enemyIt->shape.getGlobalBounds())) {
                        bulletIt = bullets.erase(bulletIt);
                        enemyIt->hp -= 1; // 체력 감소

                        if (enemyIt->isDead()) {
                            score += 10;
                            scoreText.setString("Score: " + std::to_string(score));
                            enemyIt = enemies.erase(enemyIt);
                        }
                        else {
                            ++enemyIt;
                        }

                        bulletRemoved = true;
                        break;
                    }
                    else {
                        ++enemyIt;
                    }
                }
                if (!bulletRemoved) ++bulletIt;
            }

            // 적과 플레이어 충돌 처리 (목숨 감소)
            for (auto enemyIt = enemies.begin(); enemyIt != enemies.end();) {
                if (enemyIt->shape.getGlobalBounds().intersects(player.getGlobalBounds())) {
                    enemyIt = enemies.erase(enemyIt);
                    lives--;
                    lifeText.setString("Lives: " + std::to_string(lives));
                    if (lives <= 0) {
                        isGameOver = true;
                        break;
                    }
                }
                else {
                    ++enemyIt;
                }
            }
        }

        // Drawing
        window.clear();
        window.draw(player);

        for (auto& bullet : bullets)
            window.draw(bullet.shape);
        for (auto& enemy : enemies)
            window.draw(enemy.shape);

        window.draw(scoreText);
        window.draw(lifeText);

        if (isGameOver)
            window.draw(gameOverText);

        window.display();
    }

    return 0;
}
