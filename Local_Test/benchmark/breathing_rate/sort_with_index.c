#include <stdio.h>
#include <stdlib.h>

#define NUM_SUBCARRIERS 25

int aux_array[NUM_SUBCARRIERS][2];

void sort_array(){
    for (int i = 0; i < NUM_SUBCARRIERS; i++){
        for (int j = i + 1; j < NUM_SUBCARRIERS; j++){
            if (aux_array[i][0] < aux_array[j][0]){
                int temp[2];
                temp[0] = aux_array[i][0];
                temp[1] = aux_array[i][1];
                aux_array[i][0] = aux_array[j][0];
                aux_array[i][1] = aux_array[j][1];
                aux_array[j][0] = temp[0];
                aux_array[j][1] = temp[1];
            }
        }
    }
    return;
}

int main(){
    for (int i = 0; i < NUM_SUBCARRIERS; i++){
        aux_array[i][0] = rand() % 100;
        aux_array[i][1] = i; 
    }
    // for (int i = 0; i < NUM_SUBCARRIERS; i++){
    //     printf("%d ", aux_array[i][0]);
    // }
    printf("\n");
    // for (int i = 0; i < NUM_SUBCARRIERS; i++){
    //     printf("%d ", aux_array[i][1]);
    // }
    sort_array();
    printf("\nSorted array indices: \n");
    for (int i = 0; i < NUM_SUBCARRIERS; i++){
        printf("%d ", aux_array[i][1]);
    }
}