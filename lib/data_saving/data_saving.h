#include "../../models/node.h"
#include "../../models/file_data.h"
#ifndef _DATA_SAVING_H
#define _DATA_SAVING_H

int save_data_to_file(char * path, node * head);
int load_data_from_file(char * path, node ** head);

#endif