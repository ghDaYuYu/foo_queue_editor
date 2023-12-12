#pragma once
#include "stdafx.h"

class reorder_helpers {
public:

	// moves items so that the "structure" holds
	// same as ctrl+shift+cursor move in foobar standard playlist
	static void move_items_hold_structure_reordering(bool up,
		const pfc::list_base_const_t<t_size> & indices_to_move,
		pfc::list_base_t<t_size> & new_indices, 
		pfc::list_t<t_size>& ordering, 
		t_size item_count) {

		TRACK_CALL_TEXT("reorder_helpers::move_items_hold_structure_reordering");
		DEBUG_PRINT << "move_items_hold_structure_reordering. Up?" << up;

		t_size indices_to_move_count = indices_to_move.get_count();

		DEBUG_PRINT << "move_items_hold_structure_reordering. How many items we are moving?" << indices_to_move_count;

		new_indices.remove_all();

		// We do nothing if nothing selected
		if(!indices_to_move_count) return;

		// Special cases
		if(item_count == 1) {
			new_indices.add_items(indices_to_move);
			for(t_size i = 0; i < item_count; i++) { ordering.add_item(i); } // identity
			return;
		}
		if(indices_to_move_count == item_count) {
			new_indices.add_items(indices_to_move);
			for(t_size i = 0; i < item_count; i++) { ordering.add_item(i); } // identity
			return;
		}

		PFC_ASSERT(item_count >= indices_to_move_count);

		ordering.remove_all();
		new_indices.remove_all();

		pfc::avltree_t<t_size> indices_to_move_tree(indices_to_move);

		for(t_size i = 0; i < item_count; i++) { ordering.add_item(i); }
		
		bool start_swapping = false;
		if(up) {
			for(t_size i = 0; i < item_count-1; i++) {
				bool cur_exists = indices_to_move_tree.exists(i);
				bool next_exists = indices_to_move_tree.exists(i+1);
				
				if(!cur_exists && next_exists) {
					DEBUG_PRINT << "swap(" << i << "," << i+1 << ")";
					DEBUG_PRINT << i << "Starting swapping";
					// We have 'selection' gap ending at i
					start_swapping = true;
					ordering.swap_items(i, i+1);
				} else if(start_swapping && cur_exists && !next_exists) {
					DEBUG_PRINT << "swap(" << i << "," << i+1 << ")";
					DEBUG_PRINT << i << "Ending swapping";
					// We have 'selection' ending starting at i
					// Stop swapping
					start_swapping = false;
				} else if(start_swapping) {
					DEBUG_PRINT << "swap(" << i << "," << i+1 << ")";
					// We are inside a 'selection'
					ordering.swap_items(i, i+1);
				}			
			}
		} else {
			// Same as before but backwards
			for(t_size i = item_count-1; i > 0; i--) {
				bool cur_exists = indices_to_move_tree.exists(i);
				bool next_exists = indices_to_move_tree.exists(i-1);
				
				if(!cur_exists && next_exists) {
					DEBUG_PRINT << "swap(" << i << "," << i-1 << ")";
					DEBUG_PRINT << i << "Starting swapping";
					// We have 'selection' gap ending at i
					start_swapping = true;
					ordering.swap_items(i, i-1);
				} else if(start_swapping && cur_exists && !next_exists) {
					DEBUG_PRINT << "swap(" << i << "," << i-1 << ")";
					DEBUG_PRINT << i << "Ending swapping";
					// We have 'selection' ending starting at i
					// Stop swapping
					start_swapping = false;
				} else if(start_swapping) {
					DEBUG_PRINT << "swap(" << i << "," << i-1 << ")";
					// We are inside a 'selection'
					ordering.swap_items(i, i-1);
				}	
			}
		}
		
		// 'Reorder' selection
		for(t_size i = 0; i < indices_to_move_count; i++) {
			// Find what the new position of each indice to move
			t_size new_index = ordering.find_item(indices_to_move[i]);
			PFC_ASSERT(new_index != pfc::infinite_size);
			DEBUG_PRINT << "Selection:" << indices_to_move[i] << "->" << new_index;
			new_indices.add_item(new_index);
		}
	}

	static void move_items_reordering(int moveIndex, const pfc::list_base_const_t<t_size> & indicesToMove, pfc::list_base_t<t_size> & newIndices, pfc::list_t<t_size>& ordering, int item_count) {
		TRACK_CALL_TEXT("reorder_helpers::move_items_reordering");
		t_size dragged_items_count = indicesToMove.get_count();

		newIndices.remove_all();
		
		pfc::avltree_t<t_size> dragged_items_set;
		
		// fill the 'set' with values
		for(t_size i = 0; i < dragged_items_count; i++) {
			dragged_items_set.add_item(indicesToMove[i]);
		}

		
		// Iterate each index from 0...dragpoint on the condition
		// that the index is not part of the dragged_items
		int index = 0;
		for(;index < moveIndex;
			index++) {
				if(index < 0) continue;
				t_size index_size = index;
				// ...not part of the dragged items
				if(!dragged_items_set.exists(index_size)) {
					ordering.add_item(index_size);
				}
		}

		// Insert dragged items to this position
		t_size dragged_items_newIndex = ordering.get_count();
		for(t_size i = 0; i < dragged_items_count; i++) {
			ordering.add_item(indicesToMove[i]);
			newIndices.add_item(dragged_items_newIndex);
			dragged_items_newIndex++;
		}			

		// Insert rest of the indices that are not part of the dragged items
		for(;index < item_count; index++) {
				if(index < 0) continue;
				t_size index_size = index;
				// ...not part of the dragged items
				if(!dragged_items_set.exists(index_size)) {
					ordering.add_item(index_size);
				}
		}
	}
};
