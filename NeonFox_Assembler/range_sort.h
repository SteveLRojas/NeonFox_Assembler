#define true 1


typedef struct S_RANGE
{
	uint64_t start_address;
	uint64_t end_address;
} s_range;


unsigned int range_table_determine_size(linked_instruction* instruction_head)
{
	unsigned int num_segments = 0;

	// Get first instruction
	linked_instruction* current_instruction = instruction_head->next;
	if(!current_instruction)
	{
		return num_segments;
	}

	// Get starting address
	uint64_t prev_word_address = current_instruction->word_address; 
	current_instruction = current_instruction->next;

	// Search for discontinuities 
	while(current_instruction)
	{
		if (current_instruction->word_address != (prev_word_address + 1))
			++num_segments;

		prev_word_address = current_instruction->word_address;
		current_instruction = current_instruction->next;
	}
	++num_segments;

	return num_segments;
}

void range_heapify_down(s_range* heap, unsigned int heap_size, unsigned int index)
{
	unsigned int left_index;
	unsigned int right_index;
	unsigned int largest;
	s_range temp;

	while(true)
	{
		left_index = index * 2 + 1;
		right_index = index * 2 + 2;
		largest = index;

		if((left_index < heap_size) && (heap[left_index].start_address > heap[largest].start_address))
			largest = left_index;
		if((right_index < heap_size) && (heap[right_index].start_address > heap[largest].start_address))
			largest = right_index;
		if(largest == index)
			return;

		temp = heap[index];
		heap[index] = heap[largest];
		heap[largest] = temp;
		index = largest;
	}
}

void range_heapify(s_range* elements, unsigned int size)
{
	for(unsigned int index = size - 1; index != (unsigned int)(-1); --index)
	{
		range_heapify_down(elements, size, index);
	}
	return;
}

void range_heapsort(s_range* elements, unsigned int size)
{
	s_range temp;
	range_heapify(elements, size);
	for(unsigned int end = size - 1; end != (unsigned int)(-1); --end)
	{
		temp = elements[end];
		elements[end] = elements[0];
		elements[0] = temp;
		--size;

		range_heapify(elements, size);
	}
	return;
}

s_range* range_table_build(linked_instruction* instruction_head, unsigned int num_segments)
{
	s_range* range_table = (s_range*)malloc(num_segments * sizeof(s_range));

	// Get first instruction/range
	linked_instruction* current_instruction = instruction_head->next;
	uint64_t start_word_address = current_instruction->word_address;
	uint64_t end_word_address = current_instruction->word_address;
	current_instruction = current_instruction->next;

	// Search for discontinuities 
	unsigned int index = 0;
	while(current_instruction)
	{
		if (current_instruction->word_address != (end_word_address + 1))
		{
			//do the thing
			range_table[index].start_address = start_word_address;
			range_table[index].end_address = end_word_address;

			//update the thing
			++index;
			start_word_address = current_instruction->word_address;
			end_word_address = current_instruction->word_address;
		}

		end_word_address = current_instruction->word_address;
		current_instruction = current_instruction->next;
	}
	//do the thing
	range_table[index].start_address = start_word_address;
	range_table[index].end_address = end_word_address;

	range_heapsort(range_table, num_segments);
	return range_table;
}

void range_table_print(s_range* range_table, unsigned int num_segments)
{
	for(unsigned int d = 0; d < num_segments; ++d)
	{
		printf("Segment: %u\n", d);
		printf("\t Start address: %llu\n", range_table[d].start_address);
		printf("\t End address: %llu\n", range_table[d].end_address);
	}
	return;
}

void range_table_check(s_range* range_table, unsigned int num_segments)
{
	--num_segments;
	unsigned int d = 0;
	while(d < num_segments)
	{
		uint64_t end = range_table[d].end_address;
		uint64_t start = range_table[++d].start_address;
		if(end >= start)
			printf("Warning: Segment at offset %llu overlaps with previous segment.\n", (range_table[d].start_address) / 2);
	}
	return;
}

inline void range_table_free(s_range* range_table)
{
	free(range_table);
	return;
}