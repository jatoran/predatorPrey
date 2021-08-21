#include <iostream>
#include <random>
#include <algorithm> //shuffle
#include <chrono> //system_clock
#include <thread> //sleep_until
#include <vector>

using namespace std;

class Game;
class Animal;
class Predator;
class Prey;

//GAME OPTIONS
const int WORLD_DIMENSION = 10;
const int TICK_MULTIPLE = 5;
const int REFRESH_RATE = 500;
const int HISTOGRAM_DATA_POINTS = 50;
//Type Indicators
const int PREDATOR = 1;
const int PREY = 2;
//Initial Counts
const int PREDATOR_INIT_COUNT = 5;
const int PREY_INIT_COUNT = 15;
//Breeding Rates
const int PREDATOR_BREED_RATE = 15;
const int PREY_BREED_RATE = 9;
//Max Ages
const int PREDATOR_MAX_AGE = 40;
const int PREY_MAX_AGE = 20;
//Move Speeds
const int PREDATOR_MOVESPEED = 2;
const int PREY_MOVESPEED = 1;
//Additional Predator Options
const int PREDATOR_STARVATION_TIME = 10;
const int PREDATOR_SLEEP_TIME = 3;



class Game {
    friend class Animal;
    friend class Predator;
    friend class Prey;

public:
    int NUM_TICKS = 0;
    int ANIMAL_COUNTER = 0;
    int DIRECTIONS[4]{ 1,2,3,4 }; //1-Left, 2-Up, 3-Right, 4-Down
    bool TICK_TOGGLE = false; //prevents double touch of animals per tick
    Animal* world[WORLD_DIMENSION][WORLD_DIMENSION]{}; //world grid
    int PREDATOR_COUNT = 0;
    int PREY_COUNT = 0;
    vector < pair<int, int>> histogramData;

    Game() {
        for (int x = 0; x < WORLD_DIMENSION; x++)
            for (int y = 0; y < WORLD_DIMENSION; y++)
                world[x][y] = nullptr;
    }
    int randomNumber(int min, int max);
    void initSpawn();
    void printWorld();
    void calculateTick();
    void randomShuffle(int* array);
    void end();
    void run();
    void timer(int duration);
    void generateHistogram();
    void trackPopulationData();
};

class Animal {
protected:
    Game* currGame;
    int c_ID{};
    int c_maxAge{};
    int c_breedRate{};
    int c_moveSpeed{};
    int timeToBreed{};
    int x;
    int y;

public:
    bool turnToggle{};
    bool isAsleep = false;
    int c_age = 0;

    Animal() : currGame(nullptr), x(0), y(0) {}
    Animal(Game* currGame, int x, int y, int maxAge, int breedRate, int moveSpeed) {
        this->currGame = currGame;
        this->x = x;
        this->y = y;
        c_maxAge = maxAge;
        c_breedRate = breedRate;
        c_moveSpeed = moveSpeed;
        currGame->ANIMAL_COUNTER++;
        c_ID = currGame->ANIMAL_COUNTER;
        timeToBreed = c_breedRate;
        turnToggle = currGame->TICK_TOGGLE;
    }

    void move();
    void getMoveCoords(int* x, int* y, int direction);
    void handleBreeding(int x, int y);
    bool handleAge(int x, int y);
    bool locEmpty(int x, int y);
    bool locInGrid(int x, int y);
    virtual void kill(int x, int y) {};
    virtual bool handleHunger(int x, int y) { return false; };
    virtual bool handleSleep() { return false; };
    virtual int getType() = 0;
};

class Predator : public Animal {
protected:
    int timeToEat = PREDATOR_STARVATION_TIME;
    int timeToSleep = 0;

public:
    Predator() : Animal() {}
    Predator(Game* currGame, int x, int y)
        :Animal(currGame, x, y, PREDATOR_MAX_AGE, PREDATOR_BREED_RATE, PREDATOR_MOVESPEED) {}

    int getType() { return PREDATOR; }
    void kill(int x, int y);
    bool handleHunger(int x, int y);
    bool handleSleep();
};

class Prey : public Animal {
public:
    Prey() : Animal() {}
    Prey(Game* currGame, int x, int y)
        :Animal(currGame, x, y, PREY_MAX_AGE, PREY_BREED_RATE, PREY_MOVESPEED) {}

    int getType() { return PREY; }
};

void newGame() {
    Game newGame;
    newGame.run();
    newGame.end();
}

void Game::timer(int duration) {
    this_thread::sleep_until(chrono::system_clock::now() + chrono::milliseconds(duration));
}

void Game::generateHistogram() {
    int size = histogramData.size() - 1;
    int width = HISTOGRAM_DATA_POINTS;
    double sizeWidthRatio = size / width;
    int height = 20;

    double currPred{};
    double currPrey{};
    double ratio = (PREDATOR_COUNT + PREY_COUNT) / height;

    //create array of predator to prey ratios concatenated to width of histogram
    double histogramRatioArray[HISTOGRAM_DATA_POINTS];

    //populate first and last data points of array
    //histogramRatioArray[0] = histogramData[0].first / ratio;
    histogramRatioArray[HISTOGRAM_DATA_POINTS - 1] = histogramData[size].first / ratio;

    //cout << endl << histogramData[0].first << endl;
    //populate center values of array
    for (int i = 0; i < HISTOGRAM_DATA_POINTS - 1; i++) {
        currPred = histogramData[i * sizeWidthRatio].first;
        currPrey = histogramData[i * sizeWidthRatio].second;
        ratio = (currPred + currPrey) / height;
        histogramRatioArray[i] = histogramData[i * sizeWidthRatio].first / ratio;
        //cout << currPred << " , " << currPrey << " : ";
    }

    //cout << endl;
    //for (int i = 0; i < 10; i++)
        //cout<< histogramRatioArray[i] << " , ";

    cout << endl << "Population Histogram" << endl;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (j == 0) {
                if (histogramRatioArray[j] >= i) cout << "x";
                else cout << "-";
            }
            else if (j == width - 1) {
                if (histogramRatioArray[j] >= i) cout << "x";
                else cout << "-";
            }
            else if (histogramRatioArray[j] >= i) cout << "x";
            //something in between, to make a clear line
            else cout << "-";
        }
        cout << " " << endl;
    }
}

void Game::trackPopulationData() {
    PREDATOR_COUNT = 0;
    PREY_COUNT = 0;
    for (int x = 0; x < WORLD_DIMENSION; x++) {
        for (int y = 0; y < WORLD_DIMENSION; y++) {
            if (world[x][y] != nullptr) {
                if (world[x][y]->getType() == PREDATOR) PREDATOR_COUNT++;
                else if (world[x][y]->getType() == PREY) PREY_COUNT++;
            }
        }
    }
    histogramData.push_back(make_pair(PREDATOR_COUNT, PREY_COUNT));
}

void Game::run() {
    this->initSpawn();
    this->printWorld();
    timer(400);

    while (this->PREDATOR_COUNT > 0 && this->PREY_COUNT > 0) {
        timer(REFRESH_RATE);
        for (int i = 0; i < TICK_MULTIPLE; i++)
            this->calculateTick();
        this->printWorld();
    }
    this->end();
}

void Game::end() {
    char userInput;
    this->printWorld();
    if (PREDATOR_COUNT == 0) cout << "Prey Won";
    else cout << "Predators Won";
    cout << " after " << NUM_TICKS << " Ticks";
    generateHistogram();
    cout << endl << "Run (a)gain?" << endl;
    cin >> userInput;
    if (userInput == 'a') {
        newGame();
    }
    else cout << "Bye" << endl;
}

int Game::randomNumber(int min, int max) {
    random_device rd; // obtain a random number from hardware
    mt19937 gen(rd()); // seed the generator
    uniform_int_distribution<> distr(min, max); // define the range
    return distr(gen);
}

void Game::initSpawn() {
    int x;
    int y;

    while (PREY_COUNT < PREY_INIT_COUNT) {
        x = randomNumber(0, WORLD_DIMENSION - 1);
        y = randomNumber(0, WORLD_DIMENSION - 1);
        if (world[x][y] != nullptr) continue;
        world[x][y] = new Prey(this, x, y);
        PREY_COUNT++;
    }

    while (PREDATOR_COUNT < PREDATOR_INIT_COUNT) {
        x = randomNumber(0, WORLD_DIMENSION - 1);
        y = randomNumber(0, WORLD_DIMENSION - 1);
        if (world[x][y] != nullptr) continue;
        world[x][y] = new Predator(this, x, y);
        PREDATOR_COUNT++;
    }
}

void Game::calculateTick() {
    NUM_TICKS++;
    for (int x = 0; x < WORLD_DIMENSION; x++) {
        for (int y = 0; y < WORLD_DIMENSION; y++) {
            if (world[x][y] != nullptr && world[x][y]->turnToggle == TICK_TOGGLE) {
                world[x][y]->handleBreeding(x, y);
                if (world[x][y]->handleAge(x, y)) continue;
                if (world[x][y]->getType() == PREDATOR) {
                    if (world[x][y]->handleHunger(x, y)) continue;
                    if (world[x][y]->handleSleep()) continue;
                }
                world[x][y]->turnToggle = !world[x][y]->turnToggle;
                world[x][y]->move();
            }
        }
    }
    TICK_TOGGLE = !TICK_TOGGLE;
    trackPopulationData();
}

void Game::printWorld() {
    cout << "\033[2J\033[0;0H"; //clear console
    cout << "X/x = predator | O/o = prey | * = sleeping " << endl;
    cout << "Predators: " << PREDATOR_COUNT << "| Prey: " << PREY_COUNT << " | Tick#" << NUM_TICKS << endl << endl;
    //PREDATOR_COUNT = 0;
    //PREY_COUNT = 0;
    for (int x = 0; x < WORLD_DIMENSION; x++) {
        for (int y = 0; y < WORLD_DIMENSION; y++) {
            if (world[x][y] == nullptr)
                cout << " - ";
            else if (world[x][y]->getType() == PREDATOR) {
                //PREDATOR_COUNT++;
                if (world[x][y]->c_age > PREDATOR_MAX_AGE / 2)
                    cout << " X";
                else cout << " x";
                if (world[x][y]->isAsleep == true)
                    cout << "*";
                else cout << " ";
            }
            else {
                //PREY_COUNT++;
                if (world[x][y]->c_age > PREY_MAX_AGE / 2)
                    cout << " O ";
                else cout << " o ";
            }
        }
        cout << endl;
    }
}

void Game::randomShuffle(int* array) {
    random_shuffle(array, array + 4);
}

bool Animal::handleAge(int x, int y) {
    int maxAge{};
    this->c_age++;
    if (this->getType() == PREDATOR)
        maxAge = PREDATOR_MAX_AGE;
    if (this->getType() == PREY)
        maxAge = PREY_MAX_AGE;

    if (this->c_age >= maxAge) {
        currGame->world[x][y] = nullptr;
        return true;
    }
    return false;
}

void Animal::getMoveCoords(int* x, int* y, int direction) {
    if (direction == 1) *x = *x - 1;
    else if (direction == 2) *y = *y - 1;
    else if (direction == 3) *x = *x + 1;
    else if (direction == 4) *y = *y + 1;
}

bool Animal::locEmpty(int x, int y) {
    if (currGame->world[x][y] == nullptr) return true;
    else return false;
}

bool Animal::locInGrid(int x, int y) {
    if (x < WORLD_DIMENSION && y < WORLD_DIMENSION && x >= 0 && y >= 0) return true;
    else return false;
}

void Animal::handleBreeding(int x, int y) {
    int direction;
    int newX = x;
    int newY = y;
    int attempts = 0;
    bool breedComplete = false;

    if (this->timeToBreed > 0)
        this->timeToBreed--;
    if (this->timeToBreed == 0) {
        currGame->randomShuffle(currGame->DIRECTIONS);

        while ((attempts < 4) && (breedComplete == false)) {
            direction = currGame->DIRECTIONS[attempts];
            getMoveCoords(&newX, &newY, direction);
            attempts++;

            if (this->getType() == PREY && locEmpty(newX, newY) && locInGrid(newX, newY)) {
                this->timeToBreed = PREY_BREED_RATE;
                currGame->world[newX][newY] = new Prey(currGame, newX, newY);
                breedComplete = true;
            }
            else if (this->getType() == PREDATOR && locInGrid(newX, newY)) {
                if (locEmpty(newX, newY) || currGame->world[newX][newY]->getType() == PREY) {
                    this->timeToBreed = PREDATOR_BREED_RATE;
                    currGame->world[newX][newY] = new Predator(currGame, newX, newY);
                    breedComplete = true;
                }
            }
        }
    }
}

void Animal::move() {
    int moveCount = c_moveSpeed;
    int initialX = x;
    int initialY = y;
    int attempts = 0;
    int direction;

    currGame->randomShuffle(currGame->DIRECTIONS);

    while (attempts < 4 && moveCount > 0) {
        int newX = x;
        int newY = y;

        direction = currGame->DIRECTIONS[attempts];
        getMoveCoords(&newX, &newY, direction);
        attempts++;

        if (this->getType() == PREDATOR) {
            if (!locEmpty(newX, newY) && locInGrid(newX, newY)) {
                if (currGame->world[newX][newY]->getType() == PREY) {
                    this->kill(newX, newY);
                    moveCount = 1;
                }
            }
        }

        if (locEmpty(newX, newY) && locInGrid(newX, newY)) {
            if (initialX != newX || initialX != newY) {
                currGame->world[newX][newY] = this;
                currGame->world[x][y] = nullptr;
                x = newX;
                y = newY;
                moveCount--;
                attempts = 0;
            }
        }
    }
}

bool Predator::handleHunger(int x, int y) {
    this->timeToEat--;
    if (this->timeToEat == 0) {
        currGame->world[x][y] = nullptr;
        return true;
    }
    return false;
}

void Predator::kill(int x, int y) {
    this->timeToEat = PREDATOR_STARVATION_TIME;
    this->timeToSleep = PREDATOR_SLEEP_TIME;
    this->isAsleep = true;
    currGame->world[x][y] = nullptr;
}

bool Predator::handleSleep() {
    this->timeToSleep--;
    if (this->timeToSleep > 0) {
        return true;
    }
    this->isAsleep = false;
    return false;
}

int main() {
    newGame();
    return 0;
}