#include "childpi_app.h"
#include "childpi_server.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void init_app(){
  app = malloc(sizeof(struct childpi_app));
  app->state = NOT_SIGNIN;
  init_server_stub(SERVER,SERVICE);
}

void destroy_app(){
  free(app);
  destroy_server_stub();
}


static int poll_status(const int is_ending){
  int flag = 0;
  int first = 1;

  time_t start = time(NULL);  
  while ((time(NULL) - start)<TIME_OUT) {  //Time out
    
    
    flag =  check_srvstate(is_ending);

    if(flag == -1){
      return -1;
    }

    if(flag == 1){
      return 1;
    }

    if(first){
      (is_ending==0)?printf("Waitting for other users...\n"):printf("Waiting the end of the game...");
      first--;
    }
    fflush(stdout);
    sleep(POLLING_TIME);

  }

  return -1;
}






static void print_status(){
  printf("Your sum now is %d with %d turns left\n",app->user_sum,app->max_turn - app->user_turn);
}


static void print_gainedpi(int * pis, int length){
  printf("Pi gained: %d", pis[0]);
  for(int i = 1; i<length;i++){
    printf(", %d", pis[i]);
  }
  printf("\n");
  print_status();
}

void play(){
  init_app();
  int flag = 0;
  int exit = 0;

  while (1){
    printf("\n\n\n---------------------------------------------------\n");
    switch(app->state){
      case NOT_SIGNIN:
        printf("Enter your name for playing game: ");
        scanf("%s",app->user_name);
        
        //Check if the user name is exist
        flag = sign_in(app->user_name, &app->max_turn, &app->user_sum, &app->user_turn);
        if(flag == -1){
          printf("Error happended while sending to server\n");
          exit = 1;
          break;
        }

        if(flag == 0){
          printf("Welcome %s\n", app->user_name);
          print_status();
        }

        if(flag == 1){
          printf("Login as old user %s\n", app->user_name);
          print_status();
        }

        app->state = WAITTING;
        break;
      
      case WAITTING:
        flag = poll_status(0);

        // Time out or error
        if(flag == -1){
          printf("Error happened while polling to the server\n");
          exit =1;
          break;
        }

        app->state = PLAYING;
        break;
      case PLAYING:
        // Print option
        print_status();
        printf("Choose your options:\n");
        printf("1. Get pi\n");
        printf("2. Exit\n");

        int option = 0, check = 0;
        printf("Your option is: ");
        check = scanf("%d", &option);



        //Processing
        printf("--------------\n");
        if(option == 1){
          int pis[100];
          int length = 0;
          int flag = get_pi(app->user_name, &pis[0], &length,&app->user_sum, &app->user_turn);
          if(flag == -1){
            printf("Error happened while get pi from the server\n");
            exit = 1;
            break;
          }

          print_gainedpi(pis, length);

          //Check if the user has got no turn
          if(app->user_turn >= app->max_turn){
            printf("You has got no turn. Waiting for the report...\n");
            flag = poll_status(1);
            if(flag == -1){
              printf("Error happened while polling the ending of the server\n");
              exit = 1;
              break;
            }
            app->state = ENDING;
          }
          break;
        }

        if(option == 2){
          exit = 1;
          break;
        }

        
        printf("Invalid option, please try again\n");
        break;

      case ENDING:
        printf("The game was ended.\n");
        print_status();
        
        //Print out the winner
        char name[MAX_CHAR];
        int sum = 0;
        int flag = get_winner(name,MAX_CHAR,&sum);

        if(sum > (app->user_sum)){
          printf("The winner is %s with %d points which is as higher %d than that of you.\n", name,sum,sum-app->user_sum );
          exit = 1;
          break;
        }
          
        
        if(sum == (app->user_sum)){
          if(!strcmp(name, app->user_name)){
            printf("You are the winner with %d points. Congratulation!!!\n", app->user_sum);
            exit = 1;
            break;
          }

          printf("You are the winner but ranked equally with %s. Both of you got %d points. Congratulation!!!\n", name, app->user_sum);
          exit = 1;
          break;
        }

        printf("Something goes wrong. App exit...\n");
        exit = 1;
        break;

      default:
        printf("Oops, Something goes wrong\n");
        exit = 1;
        break;
    }

    if(exit ==1){
      break;
    }
  }

  destroy_app();
}