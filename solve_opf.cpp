
#define SIZE 4
#define NUM_BUSES SIZE
#define NUM_LINES SIZE - 1
#define MAX_NUMBER_CHILDREN 3
#define MAX_NUMBER_NEIGHBORS MAX_NUMBER_CHILDREN + 2

#define DEBUG 1

#include <stdio.h>
#include <math.h>

int adj_matrix[NUM_BUSES][NUM_BUSES] = {{0,-1,0,0},{1,0,-1,-1},{0,1,0,0},{0,1,0,0}};
int neighbor_matrix[NUM_BUSES][MAX_NUMBER_NEIGHBORS];

float v_bus[NUM_BUSES] = {1.,1.,1.,1.};
float p_inj[NUM_BUSES] = {2.34,3.12,2.2,1.};
float q_inj[NUM_BUSES] = {.32,.13,.23,.32};

float P_line[NUM_LINES];
float Q_line[NUM_LINES];

float l_line[NUM_LINES];

float R_line[NUM_LINES];
float X_line[NUM_LINES];
 
struct state{
    float voltage;
    float p_inj;
    float q_inj;
    float P_line;
    float Q_line;
};

float x[NUM_BUSES][6]; // xi = [vi_x,pi_x,qi_x,Pi_x,Qi_x,li_x]
float y[NUM_BUSES][6][MAX_NUMBER_NEIGHBORS]; // yij = [vij_y,pij_y,qij_y,Pij_y,Qij_y,lij_y] \ TRIDIMENSIONAL ARRAY: NODE * VARIABLE * NEIGHBOR.

int main(){
    /* inicializacion */

    // Init 1
    for (int i = 0; i < NUM_BUSES; i++){
        v_bus[i] = 1.0;
    }

    // Init 2
    for (int i = 0; i < NUM_LINES; i++){
        P_line[i] = p_inj[i+1];
        Q_line[i] = q_inj[i+1];
    }

    // Init 3
    for (int i = 1; i < NUM_BUSES; i++){
        for (int j = 1; j < NUM_BUSES; j++){
            if (adj_matrix[i][j] == -1 ){
                float a  = P_line[i-1];
                P_line[i-1] += P_line[j - 1];
                if (DEBUG){
                    printf("i = %d, j = %d, P_line[i] = %f, P_line[j-1] = %f, P_line_new[i] = %f\n", i,j,a,P_line[j-1],P_line[i-1]) ;
                }
                Q_line[i-1] += Q_line[j - 1];
            }
        }
    }

    // Init 4
    for (int i = 0; i < NUM_LINES; i++){
        l_line[i] = (P_line[i]* P_line[i] + Q_line[i]*Q_line[i])/v_bus[i];
    }

    // Make neighbor matrix
    int c;
    neighbor_matrix[0][0] = -1; // tree root
    for (int i = 0;  i < NUM_BUSES; i++){
        c = 2;
            for (int k = 0;  k < NUM_BUSES; k++){ 
                //search for parent
                if (adj_matrix[i][k] > 0){
                    neighbor_matrix[i][0] = k;
                }
                //search for children
                if (adj_matrix[i][k] < 0){
                    neighbor_matrix[i][c] = k;
                    c = c + 1;
                }
            }
        neighbor_matrix[i][1] = i; // self
    }

    if (DEBUG){
        printf("\nNeighbors matrix:\n"); 
        for (int i = 0;  i < NUM_BUSES; i++){
            printf("Node %d: [%d,%d,%d,%d,%d]\n",i,neighbor_matrix[i][0],neighbor_matrix[i][1],neighbor_matrix[i][2],neighbor_matrix[i][3],neighbor_matrix[i][4]); 
        }
    }

    // Init state values

    // Parent
    x[0][0] = v_bus[0];
    x[0][1] = p_inj[0];
    x[0][2] = q_inj[0];
    x[0][3] = 0;
    x[0][4] = 0;
    x[0][5] = 0;
    
    // Non-parent nodes
    for (int i = 1; i < NUM_BUSES; i++){
        //[vi_x,pi_x,qi_x,Pi_x,Qi_x,li_x]
        x[i][0] = v_bus[i];
        x[i][1] = p_inj[i];
        x[i][2] = q_inj[i];
        x[i][3] = P_line[i-1];
        x[i][4] = Q_line[i-1];
        x[i][5] = l_line[i-1];
    }
    
    // Init 5
    int neighbor;
    for (int i = 0;  i < NUM_BUSES; i++){
        for (int m = 0; m < 6; m++){
            for (int j = 0; j < MAX_NUMBER_NEIGHBORS; j++){
                y[i][m][j] = 0; // init observation matrix as zeros
                neighbor = neighbor_matrix[i][j];
                if (j < 1) {
                    y[i][0][j]=x[neighbor][0]; // yij = vi_x
                }
    
                if (j > 1 & i != j){ // children
                    y[i][3][j]=x[neighbor][3]; // yij_3 = Pi_x
                    y[i][4][j]=x[neighbor][4]; // yij_4 = Qi_x
                    y[i][5][j]=x[neighbor][5]; // yij_5 = li_x
                }
    
                if (j == 1){ // self
                    y[i][0][j]=x[j][0]; // yij_0 = vi_x
                    y[i][1][j]=x[j][1]; // yij_1 = pi_x
                    y[i][2][j]=x[j][2]; // yij_2 = qi_x
                    y[i][3][j]=x[j][3]; // yij_3 = Pi_x
                    y[i][4][j]=x[j][4]; // yij_4 = Qi_x
                    y[i][5][j]=x[j][5]; // yij_5 = li_x
                }
            }   
        }
    }


    for (int i = 0; i<NUM_LINES; i++){
        printf("Linea %d | P = %f , Q = %f, l = %f \n", i + 1, P_line[i],Q_line[i],l_line[i]);
    }

    if (DEBUG){
        printf("\nState matrix:\n"); 
        for (int i = 0;  i < NUM_BUSES; i++){
            printf("Node %d: [v = %f,p = %f,q = %f,P = %f,Q = %f,l = %f]\n",i,x[i][0],x[i][1],x[i][2],x[i][3],x[i][4],x[i][5]); 
            
        }
    }

    if (DEBUG){
        printf("\nObservation matrix:\n"); 
        for (int i = 0;  i < NUM_BUSES; i++){
            for (int j = 0; j < MAX_NUMBER_NEIGHBORS; j++){
                printf("Node %d - Neighbor %d: [v = %f,p = %f,q = %f,P = %f,Q = %f,l = %f]\n",i,j,y[i][0][j],y[i][1][j],y[i][2][j],y[i][3][j],y[i][4][j],y[i][5][j]); 
            }
        }
    }


   return 0;
}