#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

void setupprefix(const char* arg0);
void perrandquit();
void customerrorquit(const char* error); 
void customerrorquit(std::string error); 
void custerrhelpprompt(const char* error);
void custerrhelpprompt(std::string error);

#endif
