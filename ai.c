#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"


void copy_state(state_t* dst, state_t* src){
	
	//Copy field
	memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

	dst->cursor = src->cursor;
	dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ){
	node_t* n = solution_node;
	while( n->parent != NULL ){
		copy_state( &(solution[n->depth]), &(n->state) );
		solution_moves[n->depth-1] = n->move;

		n = n->parent;
	}
	solution_size = solution_node->depth;
}


node_t* create_init_node(state_t* init_state){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n -> parent = NULL;
	new_n -> depth = 0;
	copy_state(&(new_n->state), init_state);
	return new_n;
}

/* cleans all the created hash table */
void clean_hashtable(HashTable table){
	int i;	
	free_stack();
	/* use ht_clear guide from https://github.com/goldsborough/hashtable/blob/master/README.md */
	ht_clear(&table);
	ht_destroy(&table);
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action){

    node_t* new_node = NULL;
	new_node = (node_t *)malloc(sizeof(node_t));
	
	// new node points to parent
	new_node->parent = n;
	// updates the depth of the node
	new_node->depth = (n->depth) + 1;
	new_node->move = action;
	copy_state(&(new_node->state), &(n->state));
	new_node->state.cursor = *selected_peg;
	
	// runs the action
    execute_move_t(&(new_node->state), &(new_node->state.cursor), action);
	return new_node;

}
void find_solution(state_t* init_state){
	HashTable table;
	
	// Choose initial capacity of PRIME NUMBER 
	// Specify the size of the keys and values you want to store once 
	ht_setup(&table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 100);
	
	// Initialize Stack
	initialize_stack();
	
	// Add the initial node
	node_t* n = create_init_node(init_state);
	stack_push(n);
	int remaining_pegs = num_pegs(init_state);
	node_t **result_node = malloc(budget*sizeof(node_t*)); ///
	// Initializes variables
	position_s position_peg;
	node_t* new_node;
	
	/* main loop */
	while (!is_stack_empty()){
		n = stack_top();
		stack_pop();
		result_node[expanded_nodes] = n; ///
		expanded_nodes++;
		/* saves solution if found a better solution */
		if (num_pegs(&(n->state)) < remaining_pegs){
			save_solution(n);
			remaining_pegs = num_pegs(&(n->state));
		}
		
		/* to check if each positions on the board can move */
		int i = 0;
		for (i; i < SIZE; i++){
			int j = 0;
			for (j; j < SIZE; j++){
				int moves = 0;
				for (moves; moves < 4; moves++){
					position_peg.x = i;
					position_peg.y = j;
					
					/* if a legal action */
					if(can_apply(&(n->state), &position_peg, moves)){
						new_node = applyAction(n, &position_peg, moves);
						/* counts generated_nodes */
						generated_nodes++;
						
						/* wins if only a peg left */
						if (won(&(new_node->state))){
							save_solution(new_node);
							remaining_pegs = num_pegs(&(new_node->state));
							
							/* frees all data if already won */
							int freed = 0; ///
							while (freed < expanded_nodes){ ///
                                free(result_node[freed]);///
                                freed++; ///
                            }
                            free(result_node);
							free(new_node);
							clean_hashtable(table);
							return;
						}
						/* if non duplicated state, add to hash table */
						if (!ht_contains(&table, new_node->state.field)){
							ht_insert(&table, new_node->state.field, new_node->state.field); 
							stack_push(new_node);
							
						}
						else{
							free(new_node);
						}
					}	
				}
			}		
		}
		/* frees the node and breaks out of the loop if expanded node exceeds bubdget */
		if (expanded_nodes >= budget){
			int freed = 0; ///
			while (freed < expanded_nodes){ ///
                                free(result_node[freed]);///
                                freed++; ///
            }
            free(result_node);
			free(new_node);
			clean_hashtable(table);
			break;
		}
	}
	clean_hashtable(table);
}