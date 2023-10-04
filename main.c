#include "stdio.h"
#include "stdlib.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL2_gfxPrimitives.h"
#include "time.h"

#include "formulas.h"
#include "wall.h"
#include "robot.h"

int done = 0;

int main(int argc, char *argv[]) {
    SDL_Window *window;
    SDL_Renderer *renderer;

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        return 1;
    }

    window = SDL_CreateWindow("Robot Maze", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, OVERALL_WINDOW_WIDTH, OVERALL_WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    window = SDL_CreateWindow("Robot Maze", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, OVERALL_WINDOW_WIDTH, OVERALL_WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, 0);

    struct Robot robot;
    struct Wall_collection *head = NULL;
    int front_centre_sensor, left_sensor, right_sensor=0;
    clock_t start_time, end_time;
    int msec;
    int crashed = 0;

    // SETUP MAZE
    // You can create your own maze here. line of code is adding a wall.
    // You describe position of top left corner of wall (x, y), then width and height going down/to right
    // Relative positions are used (OVERALL_WINDOW_WIDTH and OVERALL_WINDOW_HEIGHT)
    // But you can use absolute positions. 10 is used as the width, but you can change this.
    int name_index = 0;
    for (int i=0; i<5; i++) {
        for (int j=0; j<5; j++) {
            int xPos = ((OVERALL_WINDOW_WIDTH/2) - BLOCK_SIZE/2) + (WALL_HEIGHT+BLOCK_SIZE)*(j-2);
            int yPos = ((OVERALL_WINDOW_HEIGHT/2) - BLOCK_SIZE/2) + (WALL_HEIGHT+BLOCK_SIZE)*(i-2);
            insertAndSetFirstWall(&head, name_index++, xPos, yPos, BLOCK_SIZE, BLOCK_SIZE);
        }
    }

    // Declare a FILE pointer
    FILE *file_hori;
    FILE *file_verti;

    // Open the file for reading
    file_hori = fopen("maze_hori.txt", "r");
    char line_hori[6] = {0};  // Assuming each line had 4 char + (1 \n) and + (1 unknown?)
    file_verti = fopen("maze_verti.txt", "r");
    char line_verti[12] = {0};  // Assuming each line had 10 char + (1 \n) and + (1 unknown?)

    // Check if the file was opened successfully
    if (file_hori == NULL || file_verti == NULL) {
        printf("File could not be opened.\n");
        return 1; // Exit the program with an error code
    }

    // Read and process each line
    int row = 0;
    while (fgets(line_hori, sizeof(line_hori), file_hori)) {
        // Parse the line character by character
        for (int col = 0; line_hori[col] != '\0'; col++) {
            if (line_hori[col] == '1') {
                int xPos = ((OVERALL_WINDOW_WIDTH/2) + BLOCK_SIZE/2) + (WALL_HEIGHT+BLOCK_SIZE)*(col-2);
                int yPos = ((OVERALL_WINDOW_HEIGHT/2) - BLOCK_SIZE/2) + (WALL_HEIGHT+BLOCK_SIZE)*(floor(row/2)-2);
                if (row%2 == 1) {
                    yPos += (BLOCK_SIZE - WALL_WIDTH);
                }
                insertAndSetFirstWall(&head, name_index++, xPos, yPos, WALL_HEIGHT, WALL_WIDTH);
            }
        }

        row++;
    }

    int row2 = 0;
    while (fgets(line_verti, sizeof(line_verti), file_verti)) {
        // Parse the line character by character
        for (int col = 0; line_verti[col] != '\0'; col++) {
            if (line_verti[col] == '1') {
                int xPos = ((OVERALL_WINDOW_WIDTH/2) - BLOCK_SIZE/2) + (WALL_HEIGHT+BLOCK_SIZE)*(floor(col/2)-2);
                int yPos = ((OVERALL_WINDOW_HEIGHT/2) + BLOCK_SIZE/2) + (WALL_HEIGHT+BLOCK_SIZE)*(row2-2);
                if (col%2 == 1) {
                    xPos += (BLOCK_SIZE - WALL_WIDTH);
                }
                insertAndSetFirstWall(&head, name_index++, xPos, yPos, WALL_WIDTH, WALL_HEIGHT);
            }
        }

        row2++;
    }

    // Close the file
    fclose(file_hori);
    fclose(file_verti);
    
    /* old default maze
    insertAndSetFirstWall(&head, 1,  OVERALL_WINDOW_WIDTH/2, OVERALL_WINDOW_HEIGHT/2, 10, OVERALL_WINDOW_HEIGHT/2);
    insertAndSetFirstWall(&head, 2,  OVERALL_WINDOW_WIDTH/2-100, OVERALL_WINDOW_HEIGHT/2+100, 10, OVERALL_WINDOW_HEIGHT/2-100);
    insertAndSetFirstWall(&head, 3,  OVERALL_WINDOW_WIDTH/2-250, OVERALL_WINDOW_HEIGHT/2+100, 150, 10);
    insertAndSetFirstWall(&head, 4,  OVERALL_WINDOW_WIDTH/2-150, OVERALL_WINDOW_HEIGHT/2, 150, 10);
    insertAndSetFirstWall(&head, 5,  OVERALL_WINDOW_WIDTH/2-250, OVERALL_WINDOW_HEIGHT/2-200, 10, 300);
    insertAndSetFirstWall(&head, 6,  OVERALL_WINDOW_WIDTH/2-150, OVERALL_WINDOW_HEIGHT/2-100, 10, 100);
    insertAndSetFirstWall(&head, 7,  OVERALL_WINDOW_WIDTH/2-250, OVERALL_WINDOW_HEIGHT/2-200, 450, 10);
    insertAndSetFirstWall(&head, 8,  OVERALL_WINDOW_WIDTH/2-150, OVERALL_WINDOW_HEIGHT/2-100, 250, 10);
    insertAndSetFirstWall(&head, 9,  OVERALL_WINDOW_WIDTH/2+200, OVERALL_WINDOW_HEIGHT/2-200, 10, 300);
    insertAndSetFirstWall(&head, 10,  OVERALL_WINDOW_WIDTH/2+100, OVERALL_WINDOW_HEIGHT/2-100, 10, 300);
    insertAndSetFirstWall(&head, 11,  OVERALL_WINDOW_WIDTH/2+100, OVERALL_WINDOW_HEIGHT/2+200, OVERALL_WINDOW_WIDTH/2-100, 10);
    insertAndSetFirstWall(&head, 12,  OVERALL_WINDOW_WIDTH/2+200, OVERALL_WINDOW_HEIGHT/2+100, OVERALL_WINDOW_WIDTH/2-100, 10);
    insertAndSetFirstWall(&head, 13, 0, 0, OVERALL_WINDOW_WIDTH/2-100, 10);
    */

    setup_robot(&robot);
    updateAllWalls(head, renderer);

    SDL_Event event;
    while(!done){
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        //Move robot based on user input commands/auto commands
        if (robot.auto_mode == 1)
            robotAutoMotorMove(&robot, front_centre_sensor, left_sensor, right_sensor);
        robotMotorMove(&robot, crashed);

        //Check if robot reaches endpoint. and check sensor values
        if (checkRobotReachedEnd(&robot, OVERALL_WINDOW_WIDTH, OVERALL_WINDOW_HEIGHT/2+100, 10, 100)){
            end_time = clock();
            msec = (end_time-start_time) * 1000 / CLOCKS_PER_SEC;
            robotSuccess(&robot, msec);
        }
        else if(crashed == 1 || checkRobotHitWalls(&robot, head)){
            robotCrash(&robot);
            crashed = 1;
        }
        //Otherwise compute sensor information
        else {
            front_centre_sensor = checkRobotSensorFrontCentreAllWalls(&robot, head);
            if (front_centre_sensor>0)
                printf("Getting close on the centre. Score = %d\n", front_centre_sensor);

            left_sensor = checkRobotSensorLeftAllWalls(&robot, head);
            if (left_sensor>0)
                printf("Getting close on the left. Score = %d\n", left_sensor);

            right_sensor = checkRobotSensorRightAllWalls(&robot, head);
            if (right_sensor>0)
                printf("Getting close on the right. Score = %d\n", right_sensor);
        }
        robotUpdate(renderer, &robot);
        updateAllWalls(head, renderer);

        // Check for user input
        SDL_RenderPresent(renderer);
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                done = 1;
            }
            const Uint8 *state = SDL_GetKeyboardState(NULL);
            if(state[SDL_SCANCODE_UP] && robot.direction != DOWN){
                robot.direction = UP;
            }
            if(state[SDL_SCANCODE_DOWN] && robot.direction != UP){
                robot.direction = DOWN;
            }
            if(state[SDL_SCANCODE_LEFT] && robot.direction != RIGHT){
                robot.direction = LEFT;
            }
            if(state[SDL_SCANCODE_RIGHT] && robot.direction != LEFT){
                robot.direction = RIGHT;
            }
            if(state[SDL_SCANCODE_SPACE]){
                setup_robot(&robot);
            }
            if(state[SDL_SCANCODE_RETURN]){
                robot.auto_mode = 1;
                start_time = clock();
            }
        }

        SDL_Delay(120);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    printf("DEAD\n");
}
