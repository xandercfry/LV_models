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
void initialize_array(int*** array,  int size, double predator_density, double prey_density,  int site_capacity);
void update_array(int*** array, int size, int* N, double birth_prob, double death_prob, double prey_diffusion_prob, double predator_diffusion_prob, double predation_prob, int site_capacity);
int* pick_particle(int ***array, int size, int* N);
int* diffusion_site(int ***array, int size, int xcurrent, int ycurrent, int site_capacity);
int* birth_site(int ***array, int size, int xcurrent, int ycurrent, int site_capacity);
int* predation_site(int ***array, int size, int xcurrent, int ycurrent, int site_capacity);

int main() {
    srand(time(nullptr));
    /* assumptions: homogenous board, square, two species (predator & prey),
     *
     * potential issues: places all predators before prey
     */
    const int size = 256;
    const int MC_steps=1000;
    // what actually matters is the ratios of the rates
    const double death_rate = 0.2;
    const double birth_rate = 0.4;
    const double predation_rate = 0.25;
    const double pred_diffusion_rate = 0.3;
    const double prey_diffusion_rate = 0.2;
    const double death_prob = death_rate/(death_rate+predation_rate+pred_diffusion_rate);
    const double birth_prob = birth_rate/(birth_rate+prey_diffusion_rate);
    const double predation_prob = predation_rate/(predation_rate+death_rate+pred_diffusion_rate);
    const double pred_diffusion_prob = pred_diffusion_rate/(predation_rate+pred_diffusion_rate+death_rate);
    const double prey_diffusion_prob = prey_diffusion_rate/(prey_diffusion_rate+birth_rate);
    const double prey_density = 3;
    const double predator_density =2;
    const int site_capacity = 5;
    if (prey_density+predator_density > site_capacity) { // catch case
        cout << "The total density is too high " << endl;
        return 1;
    }

    int N[2];
    N[0] = predator_density*size*size; // predators in first column
    N[1] = prey_density*size*size; // prey in second column
    int ***array = new int **[2]; // initialize array of zeros (2 x size x size)
    for (int s = 0; s < 2; s++) {
        array[s] = new int *[size];
        for (int x = 0; x < size; x++) {
            array[s][x] = new int[size];
            for (int y = 0; y < size; y++) {
                array[s][x][y] = 0;
            }
        }
    }
    initialize_array(array, size, predator_density, prey_density, site_capacity);
    for (int t = 0; t < MC_steps; t++) {
        cout << (double) N[0] / (size * size) << " " << (double) N[1] / (size * size) << endl;
        update_array(array, size, N, birth_prob, death_prob, prey_diffusion_prob, pred_diffusion_prob, predation_prob, site_capacity);
    }
    return 0;
}

int random_integer(int sides) { // returns values from 1 to sides
    int roll = (rand() % sides) + 1;
    return roll;
}

void initialize_array(int*** array, const int size, const double predator_density, const double prey_density, const int site_capacity) {
    int num_prey = prey_density*size*size;
    int num_predator = predator_density*size*size;
    for (int i = 1; i <= num_predator; i++){
        int row = random_integer(size)-1;
        int column = random_integer(size)-1;
        if (array[0][row][column] < site_capacity) {
            array[0][row][column] = array[0][row][column]+1;
        }else{
            i--;
        }
    }
    for (int i =1; i <= num_prey; i++){
        int row = random_integer(size)-1;
        int column = random_integer(size)-1;
        if (array[0][row][column]+array[1][row][column] < site_capacity){
            array[1][row][column] = array[1][row][column]+1;
        }else{
            i--;
        }
    }
}

void update_array(int*** array, const int size, int* N, const double birth_prob, const double death_prob, const double prey_diffusion_prob, const double predator_diffusion_prob, const double predation_prob, const int site_capacity) {
    //cout << "updated populations: " << N[0] << "  " << N[1] << endl;
    int moves = N[0]+N[1];
    for (int i=1;i<=moves;i++){
       // cout << i << " ";
        if (N[0]+N[1] == 0){ // checks for absorbing state
            return;
        }
        int* site = pick_particle(array, size, N);
        int x = site[1];
        int y = site[2];
        double roll = dist(rng);
        if (site[0]==0){
            //predator
            if (roll < predator_diffusion_prob){
                // diffuses
                int* destination = diffusion_site(array, size, x, y, site_capacity);
                if (destination == nullptr){ // catch case for if all neighboring sites are full
                } else{
                    int new_x = destination[0];
                    int new_y = destination[1];
                    array[0][x][y]=array[0][x][y]-1;
                    array[0][new_x][new_y] = array[0][new_x][new_y]+1;
                }
            } else if (roll < predator_diffusion_prob+predation_prob){
                // predation
                int* predation_location = predation_site(array, size, x, y, site_capacity);
                if (predation_location==nullptr){
                } else {
                    int pred_x = predation_location[0];
                    int pred_y = predation_location[1];
                    array[1][pred_x][pred_y] = array[1][pred_x][pred_y] - 1;
                    array[0][pred_x][pred_y] = array[0][pred_x][pred_y]+1;
                    N[0] = N[0] + 1;
                    N[1] = N[1] - 1;
                }
            } else {
                // death
                array[0][x][y]=array[0][x][y]-1;
                N[0]=N[0]-1;
            }
        } else{
            //prey
            if (roll < prey_diffusion_prob){
                // diffuses
                int* destination = diffusion_site(array, size, x, y, site_capacity);
                if (destination == nullptr){ // catch case for if all neighboring sites are full
                } else{
                    int new_x = destination[0];
                    int new_y = destination[1];
                    array[1][x][y]=array[1][x][y]-1;
                    array[1][new_x][new_y] = array[1][new_x][new_y]+1;
                }
            } else{
                // birth
                int* birth_location = birth_site(array, size, x, y, site_capacity);
                if (birth_location == nullptr) {
                } else {
                    int birth_x = birth_location[0];
                    int birth_y = birth_location[1];
                    array[1][birth_x][birth_y]=array[1][birth_x][birth_y]+1;
                    N[1]=N[1]+1;
                }
            }
        }
    }
}

int* predation_site(int ***array, const int size, int xcurrent, int ycurrent, int site_capacity) {
    static int site[2];

    int right = xcurrent == (size - 1) ? 0 : xcurrent + 1;
    /* ^This line checks if x is equal to size - 1. If it is, it means x is at the right boundary of the array,
     * so the value of right is set to 0. Otherwise, x is incremented by 1, and that value is assigned to right. */
    int left = xcurrent == 0 ? size - 1 : xcurrent - 1;
    int up = ycurrent == (size - 1) ? 0 : ycurrent + 1;
    int down = ycurrent == 0 ? size - 1 : ycurrent - 1;

    int prey_total = array[1][xcurrent][ycurrent]+array[1][right][ycurrent]+array[1][left][ycurrent]+array[1][xcurrent][up]+array[1][xcurrent][down];
    if (prey_total == 0){
        return nullptr;
    }

    int i=0;
    while (i==0) {
        int roll = random_integer(5);
        switch (roll){
            case 1:
                //same cell
                if (array[1][xcurrent][ycurrent]==0){
                } else{
                    site[0]=xcurrent;
                    site[1]=ycurrent;
                    i=1;
                }
                break;
            case 2:
                // up
                if (array[1][xcurrent][up]==0){
                } else{
                    site[0]=xcurrent;
                    site[1]=up;
                    i=1;
                }
                break;
            case 3:
                //down
                if (array[1][xcurrent][down]==0){
                } else{
                    site[0]=xcurrent;
                    site[1]=down;
                    i=1;
                }
                break;
            case 4:
                //right
                if (array[1][right][ycurrent]==0){
                } else{
                    site[0]=right;
                    site[1]=ycurrent;
                    i=1;
                }
                break;
            case 5:
                //left
                if (array[1][left][ycurrent]==0){
                } else{
                    site[0]=left;
                    site[1]=ycurrent;
                    i=1;
                }
                break;
            default:
                cout << "default case reached: predation";
        }
    }
    return site;
}

int* birth_site(int ***array, const int size, int xcurrent, int ycurrent, int site_capacity){
    static int site[2];

    int right = xcurrent == (size - 1) ? 0 : xcurrent + 1;
    /* ^This line checks if x is equal to size - 1. If it is, it means x is at the right boundary of the array,
     * so the value of right is set to 0. Otherwise, x is incremented by 1, and that value is assigned to right. */
    int left = xcurrent == 0 ? size - 1 : xcurrent - 1;
    int up = ycurrent == (size - 1) ? 0 : ycurrent + 1;
    int down = ycurrent == 0 ? size - 1 : ycurrent - 1;

    int pred_total = array[0][right][ycurrent]+array[0][left][ycurrent]+array[0][xcurrent][down]+array[0][xcurrent][up]+array[0][xcurrent][ycurrent];
    int prey_total = array[1][right][ycurrent]+array[1][left][ycurrent]+array[1][xcurrent][down]+array[1][xcurrent][up]+array[1][xcurrent][ycurrent];
    int total = pred_total+prey_total;
    if (total == site_capacity*5) {
        return nullptr;
    }

    int i = 0;
    while (i==0){
        int cell = random_integer(5);
        switch (cell){
            case 1:
                //right
                if (array[0][right][ycurrent]+array[1][right][ycurrent] == site_capacity) {
                } else {
                    site[0]=right;
                    site[1]=ycurrent;
                    i=1;
                }
                break;
            case 2:
                // left
                if (array[0][left][ycurrent]+array[1][left][ycurrent] == site_capacity) {
                } else {
                    site[0]=left;
                    site[1]=ycurrent;
                    i=1;
                }
                break;
            case 3:
                //up
                if (array[0][xcurrent][up]+array[1][xcurrent][up] == site_capacity) {
                } else {
                    site[0]=xcurrent;
                    site[1]=up;
                    i=1;
                }
                break;
            case 4:
                //down
                if (array[0][xcurrent][down]+array[1][xcurrent][down] == site_capacity) {
                } else {
                    site[0]=xcurrent;
                    site[1]=down;
                    i=1;
                }
                break;
            case 5:
                // same cell
                if (array[0][xcurrent][ycurrent]+array[1][xcurrent][ycurrent] == site_capacity){
                } else {
                    site[0]=xcurrent;
                    site[1]=ycurrent;
                    i=1;
                }
                break;
        }
    }
    return site;
}

int* diffusion_site(int ***array, const int size, int xcurrent, int ycurrent, int site_capacity){
    static int site[2];
    int right = xcurrent == (size - 1) ? 0 : xcurrent + 1;
    /* ^This line checks if x is equal to size - 1. If it is, it means x is at the right boundary of the array,
     * so the value of right is set to 0. Otherwise, x is incremented by 1, and that value is assigned to right. */
    int left = xcurrent == 0 ? size - 1 : xcurrent - 1;
    int up = ycurrent == (size - 1) ? 0 : ycurrent + 1;
    int down = ycurrent == 0 ? size - 1 : ycurrent - 1;

    int pred_total = array[0][right][ycurrent]+array[0][left][ycurrent]+array[0][xcurrent][down]+array[0][xcurrent][up];
    int prey_total = array[1][right][ycurrent]+array[1][left][ycurrent]+array[1][xcurrent][down]+array[1][xcurrent][up];
    int total = pred_total+prey_total;
    if (total == site_capacity*4) {
        return nullptr;
    }

    int i=0;
    while (i==0){


        int direction = random_integer(4);
        switch (direction){
            case 1:
                //right
                if (array[0][right][ycurrent]+array[1][right][ycurrent] == site_capacity) {
                } else {
                    site[0]=right;
                    site[1]=ycurrent;
                    i=1;
                }
                break;
            case 2:
                // left
                if (array[0][left][ycurrent]+array[1][left][ycurrent] == site_capacity) {
                } else {
                    site[0]=left;
                    site[1]=ycurrent;
                    i=1;
                }
                break;
            case 3:
                //up
                if (array[0][xcurrent][up]+array[1][xcurrent][up] == site_capacity) {
                } else {
                    site[0]=xcurrent;
                    site[1]=up;
                    i=1;
                }
                break;
            case 4:
                //down
                if (array[0][xcurrent][down]+array[1][xcurrent][down] == site_capacity) {
                } else {
                    site[0]=xcurrent;
                    site[1]=down;
                    i=1;
                }
                break;
            default:
                cout << "default case reached: diffusion";
                break;
            }
    }
    return site;
}

int* pick_particle(int ***array, const int size, int* N){
    static int site[3];
    int total = N[0]+N[1];
    int roll = random_integer(total);
    int counter=0;
    for (int x = 0; x < size; x++){
        for (int y =0; y<size; y++) {
            for (int species=0;species<2;species++){
                counter = counter+array[species][x][y];
                if (counter>=roll) { // if there's a site that triggers with more than one particle, need to specify which one reacts (future code)
                    site[0] = species;
                    site[1] = x;
                    site[2] = y;
                    return site;
                }
            }
        }
    }
}