#ifndef EXECUTECOMMAND_H
#define EXECUTECOMMAND_H

void ExecuteTLST( struct commandNode* node, struct commandNode* parent, int* pipes);
void ExecuteTFIL( struct commandNode* node, struct commandNode* parent, int* pipes);
void ExecuteTPAR( struct commandNode* node, struct commandNode* parent, int* pipes);
void ExecuteTCOM( struct commandNode* node, struct commandNode* parent, int* pipes);
void ExecuteCommand( struct commandNode* node, struct commandNode* parent, int* pipes);
void OutputRedirect(struct commandNode* node, int* pipe);

extern char curPath[50];

#endif
