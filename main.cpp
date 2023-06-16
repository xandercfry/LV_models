#include <iostream>
#include <ctime>
#include <cstdlib>
#include <random>
#include <cmath>
using namespace std;

//Creating our random number generator
random_device rd{};
mt19937 rng{ rd() };
uniform_real_distribution<double> dist{ 0.0, 1.0 }; //This generates a random number between 0 and 1 Usage example: dist(rng)

int random_integer(int sides);
int* pick_particle(int ***array, int size);
int* pick_prey_site(int ***array, int size, int row, int column);
int* pick_birth_site(int ***array, int size, int row, int column);

void initialize_array(int*** array, const int length, const double density, int K) {

    int C_capacity = density/2*length*length; // carrying capacity for each species
    for (int species = 0; species < 2; species++) { // places the species on a site within respective 3rd dimension
        for (int i = 1; i <= C_capacity; i++) {
            int row = random_integer(length)-1;
            int column = random_integer(length)-1;
            if (array[0][row][column]+array[1][row][column] < K) {
                array[species][row][column] += 1;
            } else {
                i--;
            }
        }
    }/*
    for (int i = 0; i < length; i++) { // Display the array
        for (int j = 0; j < length; j++) {
            int num=array[0][i][j]+array[1][i][j];
            cout << num << " ";
        }
        cout << endl;
    }*/
}

int random_integer(int sides) { // returns values from 1 to sides
    int roll = (rand() % sides) + 1;
    return roll;
}

void update_array(int ***array,int size,int* N, double death_rate, double birth_rate, double pred_rate, double cell_limit) {
    int moves = N[0]+N[1];

    for (int i=1; i<=moves; i++){
        if (N[0]+N[1] == 0) { // checks for the absorbing state
            return;
        }
        double death_prob = death_rate/(death_rate+pred_rate);
        double pred_prob = pred_rate/(death_rate+pred_rate);
        double birth_prob = birth_rate/(birth_rate);

        int* site = pick_particle(array, size); // returns the location of a particle
        int row = site[0];
        int column = site[1];
        double roll = dist(rng);
        if (array[0][row][column] == 1){
            // it's a predator
            if (roll < 0.5) {
                // death attempt
                if (roll<death_prob){
                    //predator dies
                    array[0][row][column]=array[0][row][column]-1;
                    N[0]=N[0]-1;
                }
            } else {
                //predation attempt - CHECK FOR NEARBY PREY
                int* prey_site = pick_prey_site(array, size, row, column); // return the location of a neighboring prey
                if (prey_site != nullptr) {
                    int prey_row = prey_site[0];
                    int prey_column = prey_site[1];
                    if (roll < pred_prob) {
                        //successful predation
                        array[1][prey_row][prey_column] = array[1][prey_row][prey_column] - 1;
                        array[0][prey_row][prey_column] = array[0][prey_row][prey_column] + 1;
                        N[0] = N[0] + 1;
                        N[1] = N[1] - 1;
                    }
                }
            }
        } else {
            // it's a prey & therefore birth attempt - CHECK FOR NEARBY OPEN CELL
            int* birth_site = pick_birth_site(array, size, row, column);
            if (birth_site != nullptr) {
                int birth_row = birth_site[0];
                int birth_column = birth_site[1];
                array[1][birth_row][birth_column] = array[1][birth_row][birth_column] + 1;
                N[1] = N[1] + 1;
            }
        }
    }
}

int* pick_birth_site(int ***array, int size, int row, int column) {
    static int site[2];

    int right = column == (size - 1) ? 0 : column + 1;
    /* ^This line checks if x is equal to size - 1. If it is, it means x is at the right boundary of the array,
     * so the value of right is set to 0. Otherwise, x is incremented by 1, and that value is assigned to right. */
    int left = column == 0 ? size - 1 : column - 1;
    int up = row == (size - 1) ? 0 : row + 1;
    int down = row == 0 ? size - 1 : row - 1;

    int variable=0;

    if (array[1][row][right]+array[0][row][right]!=0){
        variable = variable+1;
    }
    if (array[1][row][left]+array[0][row][left]!=0){
        variable = variable+10;
    }
    if (array[1][up][column]+array[0][up][column]!=0){
        variable = variable+100;
    }
    if (array[1][down][column]+array[0][down][column]!=0){
        variable = variable+1000;
    }
    if (variable == 1111){ //checks if all are full
        return NULL;
    }
    int loopcase = 0;
    do {
        int cell=random_integer(4);
        switch (cell){
            case 1:
                if (array[0][row][right] + array[1][row][right]==0){
                    site[0]=row;site[1]=right;
                    loopcase=1;
                }
                break;
            case 2:
                if (array[0][row][left] + array[1][row][left]==0){
                    site[0]=row;site[1]=left;
                    loopcase=1;
                }
                break;
            case 3:
                if (array[0][up][column] + array[1][up][column]==0){
                    site[0]=up;site[1]=column;
                    loopcase=1;
                }
                break;
            case 4:
                if (array[0][down][column] + array[1][down][column]==0){
                    site[0]=down;site[1]=column;
                    loopcase=1;
                }
                break;
            default:
                cout << "the code broke.";
                break;
        }
    }while(loopcase==0);
    return site;
}

int* pick_prey_site(int ***array, int size, int row, int column) {
    // in the future, make sure there isn't a predator and prey on the site; this could run into issues with carrying_capacity
    static int site[2];

    int right = column == (size - 1) ? 0 : column + 1;
    /* ^This line checks if x is equal to size - 1. If it is, it means x is at the right boundary of the array,
     * so the value of right is set to 0. Otherwise, x is incremented by 1, and that value is assigned to right. */
    int left = column == 0 ? size - 1 : column - 1;
    int up = row == (size - 1) ? 0 : row + 1;
    int down = row == 0 ? size - 1 : row - 1;

    int direction = ceil(dist(rng) * (array[1][row][right] + array[1][row][left] + array[1][up][column] + array[1][down][column]));

    if (array[1][row][right] + array[1][row][left] + array[1][up][column] + array[1][down][column]==0) {return NULL;}

    if (direction <= array[1][row][right]) {
        site[0] = row;
        site[1] = right;
        return site;
    } else if (direction <= array[1][row][right] + array[1][row][left]) {
        site[0] = row;
        site[1] = left;
        return site;
    } else if (direction <= array[1][row][right] + array[1][row][left] + array[1][up][column]) {
        site[0] = up;
        site[1] = column;
        return site;
    } else {
        site[0] = down;
        site[1] = column;
        return site;
    }
}

int* pick_particle(int ***array, int size){
    for (int i = 0; i<1; i++) { //picks a random cell and if there's a particle, it moves on - OPTIMIZE LATER
        int row = random_integer(size)-1;
        int column = random_integer(size)-1;
        static int site[2];
        site[0]=row;
        site[1]=column;
        int sum = array[0][row][column]+array[1][row][column];
        if (sum == 0){
            i=i-1;
        } else {
            return site;
        }
    }
}

int main() {
    srand(time(nullptr));
    int size = 300;
    int MC_steps = 1000;
    const double density = 0.90; // total density of particles (predator and prey); must be even number
    const double death_rate = 0.1;
    const double birth_rate = 1;
    const double pred_rate = 1;
    int cell_limit = 1; // this must be 1 for the current code
    int N[2];
    N[0] = density / 2 * size * size; // N[0] is predators; N[1] is prey
    N[1] = density / 2 * size * size;
    int ***array = new int **[2];
    for (int s = 0; s < 2; s++) {
        array[s] = new int *[size];
        for (int x = 0; x < size; x++) {
            array[s][x] = new int[size];
            for (int y = 0; y < size; y++) {
                array[s][x][y] = 0;
            }
        }
    }
    initialize_array(array, size, density, cell_limit);
    for (int t = 0; t < MC_steps; t++) {
        cout << (double) N[0] / (size * size) << " " << (double) N[1] / (size * size) << endl;
        update_array(array, size, N, death_rate, birth_rate, pred_rate, cell_limit);
    }
    return 0;
}