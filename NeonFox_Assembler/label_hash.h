typedef struct LABEL_NODE
{
	char* key;
	uint64_t value;
	struct LABEL_NODE* next;
} label_node;

label_node* label_map = NULL;
unsigned int label_map_size;

unsigned long hash(unsigned char* str)
{
    unsigned long hash = 5381;
    unsigned char c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void insert_label(char* label, unsigned long n_line, char* s_source_file, uint64_t address)
{
	unsigned int index = (unsigned int)(hash((unsigned char*)label) % (unsigned long)label_map_size);
	if(label_map[index].key)	//handle collision
	{
		label_node* current_node = label_map + index;
		label_node* last_node;
		while(current_node)	//advance to last node
		{
			if(!strcmp(label, current_node->key))
			{
				fprintf(stderr, "Label [%s] at line: %lu in file %s has already been declared\n", label, n_line, s_source_file);
				exit(1);
			}
			last_node = current_node;
			current_node = current_node->next;
		}
		//create new node
		last_node->next = (label_node*)malloc(sizeof(label_node));
		current_node = last_node->next;
		current_node->key = label;
		current_node->next = NULL;
		current_node->value = address;
	}
	else	//no collision
	{
		label_map[index].key = label;
		label_map[index].value = address;
	}
	return;
}

void build_label_map(molecule* molecule_head, unsigned int num_labels)
{
	label_map_size = num_labels + (num_labels >> 1);
	label_map = (label_node*)malloc(label_map_size * sizeof(label_node));
	for(unsigned int d = 0; d < label_map_size; ++d)
	{
		label_map[d].key = NULL;
		label_map[d].next = NULL;
	}

	molecule* current_molecule = molecule_head;
	while(1)
	{
		current_molecule = current_molecule->next;
		if(current_molecule == NULL)
			break;
		if(current_molecule->s_label)
		{
			uint64_t address = current_molecule->word_address;
			insert_label(current_molecule->s_label, current_molecule->n_source_line, current_molecule->s_source_file, address);
		}
	}
	return;
}

uint64_t get_label_value(char* label, unsigned long n_line, char* s_source_file)
{
	unsigned int index = (unsigned int)(hash((unsigned char*)label) % (unsigned long)label_map_size);
	if(label_map[index].key)
	{
		label_node* current_node = label_map + index;
		while(current_node)
		{
			if(!strcmp(label, current_node->key))
			{
				return current_node->value;
			}
			current_node = current_node->next;
		}
	}
	fprintf(stderr, "Unable to find label [%s] at line: %lu in file %s\n", label, n_line, s_source_file);
	exit(1);
}

void free_label_map()
{
	for(unsigned long d = 0; d < label_map_size; d++)
	{
		label_node* next_to_free = label_map[d].next;
		while(next_to_free)
		{
			label_node* to_free = next_to_free;
			next_to_free = to_free->next;
			free(to_free);
		}
	}
	free(label_map);
	return;
}
